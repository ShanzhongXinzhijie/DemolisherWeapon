/*!
 * @brief	プリミティブ用のシェーダー。
 */

static const float PI = 3.14159265358979323846;

static const float subsurface = 0.0f;
static const float specular = 0.5f;
static const float specularTint = 0.0f;
static const float sheen = 0.0f;
static const float sheenTint = 0.5f;
static const float clearcoat = 0.0f;
static const float clearcoatGloss = 1.0f;
static const float anisotropic = 0.5f;

float sqr(float x) { return x*x; }

float Beckmann(float m, float t)
{
	float M = m*m;
	float T = t*t;
	return exp((T-1)/(M*T)) / (M*T*T);
}

float spcFresnel(float f0, float u)
{
	// from Schlick
	return f0 + (1-f0) * pow(1-u, 5);
}

float3 BRDF( float3 L, float3 V, float3 N )
{
	float microfacet = 0.3;
	float f0 = 0.5;
	bool include_F = 0;
	bool include_G = 0;
	// compute the half float3
	float3 H = normalize( L + V );

	float NdotH = dot(N, H);
	float VdotH = dot(V, H);
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);

	
	float D = Beckmann(microfacet, NdotH);
	float F = spcFresnel(f0, VdotH);
	
	float t = 2.0 * NdotH / VdotH;
	float G = max( 0.0f, min(1.0, min(t * NdotV, t * NdotL)) );
	float m = 3.14159265 * NdotV * NdotL;
	/*
	NdotH = NdotH + NdotH;
	float G = (NdotV < NdotL) ? 
		((NdotV*NdotH < VdotH) ?
		 NdotH / VdotH :
		 oneOverNdotV)
		:
		((NdotL*NdotH < VdotH) ?
		 NdotH*NdotL / (VdotH*NdotV) :
		 oneOverNdotV);
	*/
	//if (include_G) G = oneOverNdotV;
	return max(F * D * G / m, 0.0);
}

float SchlickFresnel(float u, float f0, float f90)
{
  return f0 + (f90-f0)*pow(1.0f-u,5.0f);
}

float NormalizedDisneyDiffuse(float baseColor, float3 N, float3 L, float3 V, float roughness)
{
  float3 H = normalize(L+V);

  float energyBias = lerp(0.0, 0.5, roughness);
  float energyFactor = lerp(1.0, 1.0/1.51, roughness);

  float dotLH = saturate(dot(L,H));
 
  float dotNL = saturate(dot(N,L));
  float dotNV = saturate(dot(N,V));
  float Fd90 = energyBias + 2.0 * dotLH * dotLH * roughness;
  
  float FL = SchlickFresnel(1.0f, Fd90, dotNL);
  float FV = SchlickFresnel(1.0f, Fd90, dotNV);
  return (baseColor*FL*FV)/PI;
}



 //ディレクションライト。
struct SDirectionLight{
	float4	color;			//!<ライトのカラー。
	float3	direction;		//!<ライトの方向。
	unsigned int lightingMaterialIDGroup;	//!<ライトの影響を与える相手を識別するマテリアルIDグループ。0xFFFFFFFFですべてのオブジェクトにライトを当てる。
};

//モデル用の定数バッファ
cbuffer ModelCb : register(b0){
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

/*
//
//@brief	ライト用の定数バッファ。
//@details
// これを変更したらtkLightManagerDx12.hのSLightParamも変更するように。
//
cbuffer LightCb : register(b1)
{
	float3 eyePos 			: packoffset(c0);		//カメラの位置。
	int numDirectionLight 	: packoffset(c0.w);		//ディレクションライトの数。
	float3 ambientLight		: packoffset(c1);		//アンビエントライト。
	int numPointLight		: packoffset(c1.w);		//ポイントライトの数。
	float4 screenParam		: packoffset(c2);		// スクリーンパラメータ(near, far, screenWidth, screenHeight)
};

cbuffer MaterialParamCb : register(b2){
	int hasNormalMap;			//!<法線マップある？
	int hasSpecularMap;			//!<スペキュラマップある？
};
*/

//スキニング用の頂点データをひとまとめ。
struct SSkinVSIn{
	int4  Indices  	: BLENDINDICES0;
	float4 Weights  : BLENDWEIGHT0;
};
//頂点シェーダーへの入力。
struct SVSIn{
	float4 pos 		: POSITION;
	float3 Normal   : NORMAL;
	float3 Tangent  : TANGENT;
	float3 biNormal : BINORMAL;
	float2 uv 		: TEXCOORD0;
	SSkinVSIn skinVert;			//スキン用の頂点データ。
	
};
//ピクセルシェーダーへの入力。
struct SPSIn{
	float4 pos 			: SV_POSITION;
	float3 Normal   	: NORMAL;
	float3 Tangent		: TANGENT;
	float3 biNormal 	: BINORMAL;
	float2 uv 			: TEXCOORD0;
	float3 worldPos		: TEXCOORD1;
};

//シャドウマップ用の頂点シェーダーへの入力。
struct SShadowMapVSIn{
	float4 pos 		: POSITION;
	SSkinVSIn skinVert;
};
//シャドウマップ用のピクセルシェーダーへの入力。
struct SShadowMapPSIn{
	float4 pos			: SV_POSITION;
};
//テクスチャ。
Texture2D<float4> g_albedoMap : register(t0);	//アルベドマップ
/*
Texture2D<float4> g_normalMap : register(t1);	//法線マップ
Texture2D<float4> g_specMap : register(t2);		//スペキュラマップ。
//ボーン行列。
StructuredBuffer<float4x4> boneMatrix : register(t3);
//ディレクションライト。
StructuredBuffer<SDirectionLight> directionLight : register(t4);
*/

//サンプラステート。
sampler g_sampler : register(s0);

//スキン行列を計算する。
/*float4x4 CalcSkinMatrix(SSkinVSIn skinVert)
{
	float4x4 skinning = 0;	
	float w = 0.0f;
	[unroll]
	for (int i = 0; i < 3; i++)
	{
		skinning += boneMatrix[skinVert.Indices[i]] * skinVert.Weights[i];
		w += skinVert.Weights[i];
	}
	
	skinning += boneMatrix[skinVert.Indices[3]] * (1.0f - w);
	return skinning;
}*/

//頂点シェーダーのコア関数。
SPSIn VSMainCore(SVSIn vsIn, uniform bool hasSkin)
{
	SPSIn psIn;
	float4x4 m;
	//if( hasSkin ){
	//	m = CalcSkinMatrix(vsIn.skinVert);
	//}else{
		m = mWorld;
	//}
	psIn.pos = vsIn.pos;
	psIn.pos = mul(m, psIn.pos);
	psIn.worldPos = psIn.pos;
	psIn.pos = mul(mView, psIn.pos);
	psIn.pos = mul(mProj, psIn.pos);
	psIn.Normal = normalize(mul(m, vsIn.Normal));
	psIn.Tangent = normalize(mul(m, vsIn.Tangent));
	psIn.biNormal = normalize(mul(m, vsIn.biNormal));
	psIn.uv = vsIn.uv;

	return psIn;
}
//スキンなしメッシュの頂点シェーダー。
SPSIn VSMainNonSkin(SVSIn vsIn)
{
	return VSMainCore(vsIn, false);
}
 //テクスチャなしプリミティブ描画用の頂点シェーダー
SPSIn VSMain( SVSIn vsIn ) 
{
	return VSMainCore(vsIn, true);
}
//テクスチャなしプリミティブ描画用のピクセルシェーダー。
float4 PSMain( SPSIn psIn ) : SV_Target0
{
	//法線を計算。
	float3 normal = psIn.Normal;
	//if( hasNormalMap){
	//	//法線マップから法線を引っ張ってくる。		
	//	float3 binSpaceNormal = g_normalMap.Sample(g_sampler, psIn.uv).xyz;
	//	binSpaceNormal = (binSpaceNormal * 2.0f)- 1.0f;
	//	normal = psIn.Tangent * binSpaceNormal.x 
	//			+ psIn.biNormal * binSpaceNormal.y 
	//			+ psIn.Normal * binSpaceNormal.z; 
	//}else{
	//	normal = psIn.Normal;
	//}

	//ランバート拡散反射
	//float3 lig = 0;
	//float3 toEye = normalize(eyePos - psIn.worldPos);
	//float metaric = 0.0f;
	//float4 albedoColor = g_albedoMap.Sample(g_sampler, psIn.uv);
	//if(hasSpecularMap){
	//	//スペキュラマップがある。
	//	metaric = g_specMap.Sample( g_sampler, psIn.uv).a;
	//}
	//for( int ligNo = 0; ligNo < numDirectionLight; ligNo++ ){

	//	float3 baseColor = max( dot( normal, -directionLight[ligNo].direction ), 0.0f ) * directionLight[ligNo].color;
	//	lig += NormalizedDisneyDiffuse(baseColor, normal, -directionLight[ligNo].direction, toEye, 0.5f);
	//	//スペキュラ反射
	//	lig += BRDF(-directionLight[ligNo].direction, toEye, normal) * directionLight[ligNo].color.xyz * metaric * directionLight[ligNo].color.w;
	//}
	//lig += ambientLight;
	
	float4 finalColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	//finalColor.xyz = albedoColor.xyz * lig;

	//finalColor.rgb *= saturate(dot(normal, float3(1, 0, 0)));

	return finalColor;
}
//スキンなしモデルのシャドウマップ書き込み用の頂点シェーダー。
SShadowMapPSIn VSMainNonSkinShadowMap( SShadowMapVSIn vsIn )
{
	SShadowMapPSIn psIn;
	//psIn.pos = mul(mWorld, vsIn.pos);
	//psIn.pos = mul(mView, psIn.pos);
	//psIn.pos = mul(mProj, psIn.pos);
	return psIn;
}
SShadowMapPSIn VSMainSkinShadowMap( SShadowMapVSIn vsIn )
{
	SShadowMapPSIn psIn;
	//float4x4 skinMatrix = CalcSkinMatrix(vsIn.skinVert);
	//psIn.pos = mul(skinMatrix, vsIn.pos);
	//psIn.pos = mul(mView, psIn.pos);
	//psIn.pos = mul(mProj, psIn.pos);
	return psIn;
}
//シャドウマップ書き込み用のピクセルシェーダー。
float4 PSMainShadowMap( SShadowMapPSIn psIn) : SV_Target0
{
	return psIn.pos.z / psIn.pos.w;
}