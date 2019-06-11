#include"model.fx"

#if defined(INSTANCING)
//インスタンシング用テクスチャインデックス
StructuredBuffer<int2> InstancingImposterTextureIndex : register(t7);
#endif

///////////////////////////////////////////////////////////////////////
// クォータニオンの計算												 //
// https://gist.github.com/mattatz/40a91588d5fb38240403f198a938a593  //
///////////////////////////////////////////////////////////////////////
static const float PI = 3.14159265359f;
static const float PI2 = PI * 2.0f;
// Quaternion multiplication
// http://mathworld.wolfram.com/Quaternion.html
float4 qmul(float4 q1, float4 q2)
{
	return float4(
		q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
		q1.w * q2.w - dot(q1.xyz, q2.xyz)
		);
}
// Vector rotation with a quaternion
// http://mathworld.wolfram.com/Quaternion.html
float3 rotate_vector(float3 v, float4 r)
{
	float4 r_c = r * float4(-1, -1, -1, 1);
	return qmul(r, qmul(float4(v, 0), r_c)).xyz;
}
// A given angle of rotation about a given axis
float4 rotate_angle_axis(float angle, float3 axis)
{
	float sn = sin(angle * 0.5);
	float cs = cos(angle * 0.5);
	return float4(axis * sn, cs);
}

//インポスターの出力
struct PSOutput_RenderImposter {
	float4 albedo		: SV_Target0;		//アルベド
	float3 normal		: SV_Target1;		//法線
	float4 lightingParam: SV_Target2;		//ライティング用パラメーター
};
PSOutput_RenderImposter PSMain_RenderImposter(PSInput In)
{
	PSOutput_RenderImposter finalOut = (PSOutput_RenderImposter)0;
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	AlbedoRender(In, Out);

	//αテスト
	if (Out.albedo.a > 0.5f) {
		Out.albedo.a = 1.0f;//半透明無効
	}
	else {
		discard;//描画しない
	}

	NormalRender(In, Out);

	ParamRender(In, Out);

	finalOut.albedo = Out.albedo;
	finalOut.normal = Out.normal; finalOut.normal.xy *= -1.0f; finalOut.normal *= 0.5f; finalOut.normal += 0.5f;
	finalOut.lightingParam.x = Out.lightingParam.x*0.05f;	//エミッシブ
	finalOut.lightingParam.w = Out.lightingParam.y;	//ライティングするか?
	finalOut.lightingParam.y = Out.lightingParam.z;	//メタリック
	finalOut.lightingParam.z = Out.lightingParam.w;	//シャイニネス
	return finalOut;
}

//インポスターの計算
void CalcImposter(out int2 out_index, inout float3 inout_pos
#if defined(INSTANCING)
	, in uint instanceID
#endif
) {
	//インポスターの座標
#if defined(INSTANCING)
	float3 pos = float3(InstancingWorldMatrix[instanceID]._m03, InstancingWorldMatrix[instanceID]._m13, InstancingWorldMatrix[instanceID]._m23);
#else
	float3 pos = float3(mWorld._m03, mWorld._m13, mWorld._m23);
#endif

		//インポスター用インデックス計算
		float3 polyDir = normalize(camWorldPos - pos);

		//X軸回転
		float3 axisDir = polyDir; axisDir.x = length(float2(polyDir.x, polyDir.z));
		float XRot = atan2(axisDir.y, axisDir.x);
		out_index.y = (int)round(XRot / PI * imposterPartNum.y) - (int)(imposterPartNum.y / 2.0f - 0.5f);

		//Y軸回転		
		float YRot = atan2(polyDir.x, polyDir.z);
		out_index.x = (int)round(-YRot / PI2 * imposterPartNum.x) + (int)(imposterPartNum.x / 2.0f - 0.5f);

		//カメラ方向にモデルサイズ分座標ずらす
		//※埋まり防止
		//TODO ここだけプログラム側でやる
	//#if !defined(SHADOW)
	//		polyDir *= scale * texture.GetDirectionOfCameraSize(index_x, index_y);
	//#endif

		//TODO バイアス行列

		//回転		
		float r = -out_index.y * -(PI / (imposterPartNum.y - 1)) + PI * 0.5f;
		float sinr = sin(r), cosr = cos(r);
		float3x3 rotX = {1.0f,0.0f,0.0f,
						 0.0f,cosr,sinr,
						 0.0f,-sinr,cosr
						};
		r = -out_index.x * -(PI2 / (imposterPartNum.x - 1)) + PI2;
		sinr = sin(r), cosr = cos(r);
		float3x3 rotY = {cosr,sinr,0.0f, //※Z軸回転
						 -sinr,cosr,0.0f,
						 0.0f,0.0f,1.0f
						};
		//{cosr, 0.0f, -sinr,
		//	0.0f, 1.0f, 0.0f,
		//	sinr, 0.0f, cosr
		//};
		inout_pos = mul(inout_pos, rotX);
		inout_pos = mul(inout_pos, rotY);
		
/*		
		float4 rotX = rotate_angle_axis(-out_index.y * -(PI / (imposterPartNum.y - 1)) + PI * 0.5f, float3(1.0f, 0.0f, 0.0f));
		float4 rotY = rotate_angle_axis(-out_index.x * -(PI2 / (imposterPartNum.x - 1)) + PI2, float3(0.0f, 0.0f, 1.0f));
		rotX = qmul(rotY, rotX);
		inout_pos = rotate_vector(inout_pos, rotX);
*/
		//返す
		//position_return = pos + polyDir;
		//rotation_return = rot;
		//TODO
		//scale_return = scale * texture.GetModelSize()*2.0f;
}

//頂点シェーダ(通常)
PSInput VSMain_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	int2 index;
	CalcImposter(index, In.Position.xyz
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	PSInput psInput = VSMain(In
#if defined(INSTANCING)
							, instanceID
#endif 
							);

	psInput.imposterIndex = index;

	return psInput;
}
//頂点シェーダ(深度値)
ZPSInput VSMain_RenderZ_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	int2 index;
	CalcImposter(index, In.Position.xyz
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	ZPSInput psInput = VSMain_RenderZ(In
#if defined(INSTANCING)
		, instanceID
#endif 
		);

	psInput.imposterIndex = index;

	return psInput;
}

//インポスターのGBufferへの書き込み
PSOutput_RenderGBuffer PSMain_ImposterRenderGBuffer(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	/*
#if ALBEDO_MAP && NORMAL_MAP
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);
#endif
	if (In.TexCoord.x < 0.01f || In.TexCoord.x > 1.0f - 0.01f || -In.TexCoord.y < 0.01f || -In.TexCoord.y > 1.0f - 0.01f) {
		Out.albedo = float4(-In.TexCoord.y, 0, 0, 1);
		return Out;
	}
	*/	
	
	//インデックスからuv座標を算出
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;
#if defined(INSTANCING)
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
#endif

	//アルベド
#if ALBEDO_MAP
	//if(Out.albedo.a < 0.1f)
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord);
	Out.albedo *= albedoScale;//スケールをかける
#else
	discard;
#endif

	//αテスト
	if (Out.albedo.a > 0.5f) {
		Out.albedo.a = 1.0f;//半透明無効
	}
	else {
		discard;//描画しない
	}

	NormalRender(In, Out);

	PosRender(In, Out);

	//ライティング用パラメーター
#if LIGHTING_MAP
	float4 lightMap = LightingTexture.Sample(Sampler, In.TexCoord);
	Out.lightingParam.x = lightMap.x / 0.05f * emissive;	//エミッシブ
	Out.lightingParam.y = lightMap.w * isLighting;			//ライティングするか?
	Out.lightingParam.z = lightMap.y * metallic;			//メタリック
	Out.lightingParam.w = lightMap.z * shininess;			//シャイニネス
#endif

	MotionRender(In, Out);

	return Out;
}

#if defined(TEXTURE)
//インポスターのZ値出力
float4 PSMain_ImposterRenderZ(ZPSInput In) : SV_Target0
{
	//インデックスからuv座標を算出
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;
#if defined(INSTANCING)
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
#endif

	//アルベド
	float alpha = albedoTexture.Sample(Sampler, In.TexCoord).a * albedoScale.a;

	//αテスト
	if (alpha > 0.5f) {
	}
	else {
		discard;
	}

	return In.posInProj.z / In.posInProj.w + depthBias.x;
}
#endif