sampler Sampler : register(s0);
sampler NoFillteringSampler : register(s2);

//ライト
cbuffer lightCb : register(b2)
{
	float3 eyePos; 	//		: packoffset(c0);		//カメラの位置。
	int numDirectionLight;//	: packoffset(c0.w);		//ディレクションライトの数。
	int numPointLight;	//	: packoffset(c0.x);		//ポイントライトの数。
	float3 ambientLight;//		: packoffset(c1.yzw);	//アンビエントライト。
};
struct SDirectionLight {
	float3 color;
	float3 direction;
};
struct SPointLight {
	float3	position;	
	float3	color;	
	float range;
	float attenuation;
};
StructuredBuffer<SDirectionLight> directionLight : register(t100);
StructuredBuffer<SPointLight> pointLightList : register(t101);

//シャドウマップ系
#include"ShadowMapHeader.h"
cbuffer ShadowCb : register(b1) {
	float4x4 ViewProjInv;
	float4x4 mLVP[SHADOWMAP_NUM];
	float4 shadowDir[SHADOWMAP_NUM];//xyz:方向 w:バイアス
	float4 enableShadowMap[SHADOWMAP_NUM];//x:シャドウマップ有効か？ y:PCSS有効 z:解像度(横) w:解像度(縦)
	float4 cascadeArea[SHADOWMAP_NUM];//x:カスケード距離(Near) y:カスケード距離(Far) z:4000.0f/Width(平行投影カメラ) w:4000.0f/Height(平行投影カメラ)
	float4 shadowNF[SHADOWMAP_NUM];

	int boolAO;//AOを有効にするか
	int boolAmbientCube;//環境キューブマップ有効か?
};
struct HideInShadow {
	float flag[SHADOWMAP_NUM];
};
Texture2DArray<float> shadowMaps  : register(t60);
SamplerComparisonState	shadowSamplerComparisonState	: register(s1);

static const float2 blockerSampleMap[] = {
	float2(-0.00276f,-0.00276f),
	float2(-0.00276f,-0.00138f),
	float2(-0.00276f, 0.0f),
	float2(-0.00276f, 0.00138f),
	float2(-0.00276f, 0.00276f),

	float2(-0.00138f,-0.00276f),
	float2(-0.00138f,-0.00138f),
	float2(-0.00138f, 0.0f),
	float2(-0.00138f, 0.00138f),
	float2(-0.00138f, 0.00276f),

	float2(0.0f,-0.00276f),
	float2(0.0f,-0.00138f),
	float2(0.0f, 0.0f),
	float2(0.0f, 0.00138f),
	float2(0.0f, 0.00276f),

	float2(0.00138f,-0.00276f),
	float2(0.00138f,-0.00138f),
	float2(0.00138f, 0.0f),
	float2(0.00138f, 0.00138f),
	float2(0.00138f, 0.00276f),

	float2(0.00276f,-0.00276f),
	float2(0.00276f,-0.00138f),
	float2(0.00276f, 0.0f),
	float2(0.00276f, 0.00138f),
	float2(0.00276f, 0.00276f),
};
static const float2 PCSSSampleMap[] = {
	float2(-0.00138f,-0.00138f),
	float2(-0.00138f,-0.00092f),
	float2(-0.00138f,-0.00046f),
	float2(-0.00138f, 0.0f),
	float2(-0.00138f, 0.00046f),
	float2(-0.00138f, 0.00092f),
	float2(-0.00138f, 0.00138f),

	float2(-0.00092f,-0.00138f),
	float2(-0.00092f,-0.00092f),
	float2(-0.00092f,-0.00046f),
	float2(-0.00092f, 0.0f),
	float2(-0.00092f, 0.00046f),
	float2(-0.00092f, 0.00092f),
	float2(-0.00092f, 0.00138f),

	float2(-0.00046f,-0.00138f),
	float2(-0.00046f,-0.00092f),
	float2(-0.00046f,-0.00046f),
	float2(-0.00046f, 0.0f),
	float2(-0.00046f, 0.00046f),
	float2(-0.00046f, 0.00092f),
	float2(-0.00046f, 0.00138f),

	float2(0.0f,-0.00138f),
	float2(0.0f,-0.00092f),
	float2(0.0f,-0.00046f),
	float2(0.0f, 0.0f),
	float2(0.0f, 0.00046f),
	float2(0.0f, 0.00092f),
	float2(0.0f, 0.00138f),

	float2(0.00046f,-0.00138f),
	float2(0.00046f,-0.00092f),
	float2(0.00046f,-0.00046f),
	float2(0.00046f, 0.0f),
	float2(0.00046f, 0.00046f),
	float2(0.00046f, 0.00092f),
	float2(0.00046f, 0.00138f),

	float2(0.00092f,-0.00138f),
	float2(0.00092f,-0.00092f),
	float2(0.00092f,-0.00046f),
	float2(0.00092f, 0.0f),
	float2(0.00092f, 0.00046f),
	float2(0.00092f, 0.00092f),
	float2(0.00092f, 0.00138f),

	float2(0.00138f,-0.00138f),
	float2(0.00138f,-0.00092f),
	float2(0.00138f,-0.00046f),
	float2(0.00138f, 0.0f),
	float2(0.00138f, 0.00046f),
	float2(0.00138f, 0.00092f),
	float2(0.00138f, 0.00138f),
};

//デプス値を線形に変換
float LinearizeDepth(float depth, float near, float far)
{
	return (2.0 * near) / (far + near - depth * (far - near));
}

static const float DEFALT_NF = 20000.0f - 50.0f;

//シャドウマップの判定
inline float ShadowMapFunc(uint usemapnum, float4 worldpos) {

	//座標算出
	float4 lLViewPosition = mul(mLVP[usemapnum], worldpos);
	lLViewPosition.xyz /= lLViewPosition.w;

	// -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
	lLViewPosition.x = lLViewPosition.x *0.5f + 0.5f;
	// yは更に上下反転
	lLViewPosition.y = -lLViewPosition.y *0.5f + 0.5f;

	//ずらし
	float2 scale = float2( enableShadowMap[usemapnum].z / SHADOW_MAX_WIDTH, enableShadowMap[usemapnum].w / SHADOW_MAX_HEIGHT);
	lLViewPosition.xy *= scale;
	
	//バイアス
	lLViewPosition.z -= shadowDir[usemapnum].w;

	//PCSS無効
	if (!enableShadowMap[usemapnum].y) {
		return 1.0f - shadowMaps.SampleCmpLevelZero(shadowSamplerComparisonState, float3(lLViewPosition.xy, usemapnum), lLViewPosition.z);
	}

	float kekka = 0.0f;
	uint cnt = 0;
	
	//ブロッカーの深度値取得(平均)
	float blocker_z = 0.0f;
	float avg_blocker_z = 0.0f;
	[unroll]
	for (uint i = 0; i < 25; i++) {

		blocker_z = shadowMaps.Sample(NoFillteringSampler, float3(lLViewPosition.xy + blockerSampleMap[i] * scale * float2(cascadeArea[usemapnum].z, cascadeArea[usemapnum].w), usemapnum));

		if (blocker_z < lLViewPosition.z) {
			avg_blocker_z += LinearizeDepth(blocker_z, shadowNF[usemapnum].x, shadowNF[usemapnum].y);
			cnt++;
		}
	}
	if (cnt == 0) {
		return 0.0f;
	}
	else {
		avg_blocker_z /= cnt;
	}

	//半影のサイズ計算
	float maxCnt = 12.0f * (LinearizeDepth(lLViewPosition.z, shadowNF[usemapnum].x, shadowNF[usemapnum].y) - avg_blocker_z) / (avg_blocker_z * (DEFALT_NF/shadowNF[usemapnum].z));
	if (maxCnt <= 0.0f) {
		return 0.0f;
	}

	//影に入ってるか判定(PCF)
	cnt = 0;
	[unroll]
	for (uint i = 0; i < 49; i++) {
	
		kekka += 1.0f - shadowMaps.SampleCmpLevelZero(shadowSamplerComparisonState, float3(PCSSSampleMap[i] * maxCnt * scale * float2(cascadeArea[usemapnum].z, cascadeArea[usemapnum].w) + lLViewPosition.xy, usemapnum), lLViewPosition.z);

		cnt++;
	}

	kekka /= cnt;
	return kekka;
}

//G-Buffer
Texture2D<float4> albedoTexture : register(t0);
Texture2D<float4> normalMap		: register(t1);
Texture2D<float > depthMap		: register(t2);
Texture2D<float4> PosMap		: register(t3);
Texture2D<float > AoMap			: register(t4);
Texture2D<float4> lightParamTex	: register(t5);
Texture2D<float > AoMapBlur		: register(t7);
//環境キューブマップ
TextureCube<float3> AmbientCubeMap: register(t6);

struct VSDefferdInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSDefferdInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PSDefferdInput VSMain(VSDefferdInput In)
{
	PSDefferdInput psIn;
	psIn.pos = In.pos;
	psIn.uv = In.uv;
	return psIn;
}

//Z値からワールド座標を復元
float3 CalcWorldPosFromUVZ(float2 uv, float zInScreen)//, float4x4 mViewProjInv)
{
	float3 screenPos;
	screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
	screenPos.z = zInScreen;

	float4 worldPos = mul(ViewProjInv, float4(screenPos, 1.0f));
	worldPos.xyz /= worldPos.w;
	return worldPos.xyz;
}

static const float PI = 3.14f;

//スペキュラ
/*float3 NormalizedPhong(float3 specular, float power, float3 lightDir, float3 viewDir, float3 normal)
{
	float3 R = -viewDir + (2.0f * dot(normal, viewDir) * normal);
	return specular * pow(max(dot(lightDir, R), 0.0f), power) * ((power + 1.0f) / (2.0f * PI));	
}*/
//マイクロファセットの分布関数
//Blinn-Phong NDF
float3 NormalizedBlinnPhong(float power, float3 halfVec, float3 normal)
{
	return ((power + 2.0f) / (2.0f * PI)) * pow(max(0.0f, dot(normal, halfVec)), power);
}
//幾何減衰率
float G1(float3 v, float roughness, float3 normal) {
	float k = (roughness + 1.0f)*(roughness + 1.0f) / 8.0f;
	return dot(normal, v) / (dot(normal, v)*(1.0f - k) + k);
}
float G(float3 lightDir, float3 viewDir, float3 normal, float roughness){
	return G1(lightDir, roughness, normal)*G1(viewDir, roughness, normal);
}
//フレネル項
float3 Fresnel(in float3 specAlbedo, in float3 h, in float3 l) { 
	return specAlbedo + (1.0f - specAlbedo) * pow((1.0f - saturate(dot(l, h))), 5.0f);
}
//Cook-Torrance?
float3 CookTorrance(float3 lightDir, float3 viewDir, float3 normal, float3 baseColor, float shininess) {
	float3 halfVec = normalize(lightDir + viewDir);
	
	return  Fresnel(baseColor, halfVec, lightDir)
			*NormalizedBlinnPhong(pow(2.0f, 11.0f*shininess), halfVec, normal)
			*G(lightDir, viewDir, normal, 1.0f - shininess)
			/(PI*dot(normal, viewDir)*dot(normal, lightDir));
}

//ディフューズ
float3 NormalizedLambert(float3 diffuse, float3 lightDir, float3 normal)
{
	return max(diffuse * dot(normal, lightDir), 0.0f)* (1.0f / PI);
}

float4 PSMain(PSDefferdInput In) : SV_Target0
{
	float4 albedo = albedoTexture.Sample(Sampler, In.uv);

	//αテスト
	if (albedo.w == 0.0f) {
		discard;
	}

	float3 normal = normalMap.Sample(Sampler, In.uv).xyz;
	float4 viewpos = PosMap.Sample(Sampler, In.uv);
	float3 worldpos = CalcWorldPosFromUVZ(In.uv, viewpos.w);
	float4 lightParam = lightParamTex.Sample(Sampler, In.uv);
	float3 emissive = albedo.rgb*lightParam.x;

	//ライティング無効
	if (!lightParam.y) {
		return float4(albedo.rgb + emissive, albedo.w);
	}

	//シャドウマップの範囲に入っているか判定
	HideInShadow hideInShadow = (HideInShadow)0;
	[unroll]
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		if (enableShadowMap[i].x && viewpos.z > cascadeArea[i].x && viewpos.z < cascadeArea[i].y){
			hideInShadow.flag[i] = ShadowMapFunc(i, float4(worldpos, 1.0f));
			//return float4(hideInShadow.flag[i], 0, 0, 1);
		}
		/*if (!enableShadowMap[i].y && viewpos.z > cascadeArea[i].x && viewpos.z < cascadeArea[i].y) {
			if (i == 0) {
				albedo = float4(1, 0, 0, 1);
			}
			if (i == 1) {
				albedo = float4(0, 1, 0, 1);
			}
			if (i == 2) {
				albedo = float4(0, 0, 1, 1);
			}
			if (i == 3) {
				albedo = float4(0, 0, 0, 1);
			}
		}*/
	}

	//ライティング
	float3 Out = 0; 

	//視線ベクトル
	float3 viewDir = normalize(eyePos - worldpos);
	
	//ディレクションライト
	[unroll]
	for (int i = 0; i < 4; i++) {

		if (numDirectionLight == i) { break; }

		//シャドウマップの遮蔽適応
		float nothide = 1.0f;
		[unroll]
		for (int swi = 0; swi < SHADOWMAP_NUM; swi++) {
			nothide = min(nothide, saturate(1.0f - dot(shadowDir[swi].xyz, directionLight[i].direction)*-hideInShadow.flag[swi]));
		}

		Out += NormalizedLambert(albedo.xyz * (1.0f - lightParam.z), directionLight[i].direction, normal) * directionLight[i].color * nothide;		
		Out += max(0.0f,
			CookTorrance(directionLight[i].direction, viewDir, normal, lerp(float3(0.03f, 0.03f, 0.03f), albedo.xyz, lightParam.z), lightParam.w)
			* directionLight[i].color * saturate(dot(normal, directionLight[i].direction))*nothide
			);	
	}
	//ポイントライト
	[unroll]
	for (int i = 0; i < 12; i++) {

		if (numPointLight == i) { break; }

		float3 dir = pointLightList[i].position - worldpos;
		float len = length(dir);
			
		dir = normalize(dir);

		//減衰を計算する。
		float	litRate = len / pointLightList[i].range;
		float	attn = max(1.0 - litRate * litRate, 0.0);

		Out += NormalizedLambert(albedo.xyz * (1.0f - lightParam.z), dir, normal) * pointLightList[i].color * pow(attn, pointLightList[i].attenuation);
		Out += max(0.0f,
			CookTorrance(dir, viewDir, normal, lerp(float3(0.03f, 0.03f, 0.03f), albedo.xyz, lightParam.z), lightParam.w)
			* pointLightList[i].color * saturate(dot(normal, dir)) * pow(attn, pointLightList[i].attenuation)
			);	
	}	

	//AO
	float ambientOcclusion = 1.0f;
	if (boolAO) {
		ambientOcclusion = AoMapBlur.Sample(Sampler, In.uv);
		//ambientOcclusion *= lerp(AoMap.Sample(Sampler, In.uv),1.0f,0.5f);
		/*ambientOcclusion = AoMap.Sample(Sampler, In.uv + float2(-0.5f / 1280.0f, -0.5f / 720.0f));
		ambientOcclusion += AoMap.Sample(Sampler, In.uv + float2(0.5f / 1280.0f, 0.5f / 720.0f));
		ambientOcclusion += AoMap.Sample(Sampler, In.uv + float2(-0.5f / 1280.0f, 0.5f / 720.0f));
		ambientOcclusion += AoMap.Sample(Sampler, In.uv + float2(0.5f / 1280.0f, -0.5f / 720.0f));
		ambientOcclusion /= 4.0f;*/
	}
	ambientOcclusion *= (1.0f - lightParam.z);//金属なら環境光(デュフューズ)なし

	//アンビエント
	if (boolAmbientCube) {
		Out += albedo.xyz * AmbientCubeMap.SampleLevel(Sampler, normal, 9) * ambientLight * ambientOcclusion;
	}
	else {
		Out += albedo.xyz * ambientLight * ambientOcclusion;
	}

	//エミッシブを加算
	Out += emissive;

	//0.0~1.0で出力
	//Out = saturate(Out);
	return float4(Out, albedo.w);
	
}