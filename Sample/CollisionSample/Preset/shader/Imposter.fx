#ifndef DW_IMPOSTER_FX
#define DW_IMPOSTER_FX

#include"model.fx"

//モデルサイズ(カメラ方向への)
StructuredBuffer<float> ImposterSizeToCamera : register(t7);
#if defined(INSTANCING)
//インスタンシング用インポスターパラメータ
//[CInposter.h] InstancingImposterParamManager::ImposterParam
StructuredBuffer<float2> InstancingImposterParam : register(t8);
#endif

//円周率
static const float PI = 3.14159265359f;
static const float PI2 = PI * 2.0f;

//インポスターテクスチャの出力
struct PSOutput_RenderImposter {
	float4 albedo		: SV_Target0;//アルベド
	float3 normal		: SV_Target1;//法線
	float4 lightingParam: SV_Target2;//ライティング用パラメーター
    float  translucent  : SV_Target3;//トランスルーセント
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

    TranslucentRender(In, Out);

	//出力構造体にコピー
	finalOut.albedo = Out.albedo;
	finalOut.normal = Out.normal.xyz; finalOut.normal.xy *= -1.0f; finalOut.normal *= 0.5f; finalOut.normal += 0.5f;
	finalOut.lightingParam.x = Out.lightingParam.x*0.05f; //エミッシブ
	finalOut.lightingParam.w = Out.lightingParam.y;	//ライティングするか?
	finalOut.lightingParam.y = Out.lightingParam.z;	//メタリック
	finalOut.lightingParam.z = Out.lightingParam.w;	//シャイニネス
    finalOut.translucent = Out.normal.a; //トランスルーセント

	return finalOut;
}

//インポスターの計算
void CalcImposter(out int2 out_index, out float4x4 out_rotMat, out float3 out_offsetPos
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
	//インデックス計算
	out_index.y = (int)round(XRot / PI * imposterPartNum.y) - (int)(imposterPartNum.y / 2.0f - 0.5f);
	//Y軸回転		
	float YRot = atan2(polyDir.x, polyDir.z);	
	//インデックス計算
	out_index.x = (int)round(-YRot / PI2 * imposterPartNum.x) + (int)(imposterPartNum.x / 2.0f - 0.5f);

	//回転行列作成
	//X軸
	float r = -out_index.y * (PI / (imposterPartNum.y - 1)) - PI * 0.5f;
	float sinr = sin(r), cosr = cos(r);
	float4x4 rotX = {1.0f,0.0f,0.0f,0.0f, //※X軸回転
					0.0f,cosr,sinr,0.0f,
					0.0f,-sinr,cosr,0.0f,
					0.0f,0.0f,0.0f,1.0f,
					};
	//Y軸
	r = out_index.x * (PI2 / (imposterPartNum.x - 1));
	sinr = sin(r), cosr = cos(r);
	float4x4 rotY = /*{cosr,sinr,0.0f,0.0f, //※Z軸回転
					-sinr,cosr,0.0f,0.0f,
					0.0f,0.0f,1.0f,0.0f,
					0.0f,0.0f,0.0f,1.0f,
					};*/
					{cosr, 0.0f, -sinr, 0.0f, //※Y軸回転
					0.0f, 1.0f, 0.0f, 0.0f,
					sinr, 0.0f, cosr, 0.0f,
					0.0f,0.0f,0.0f,1.0f,
					};
	//回転行列の作成
	out_rotMat = mul(rotY, rotX);

	//カメラ方向にモデルサイズ分座標ずらす
	//※埋まり防止
#if defined(INSTANCING)	
	out_offsetPos = polyDir * (InstancingImposterParam[instanceID].x * ImposterSizeToCamera[(imposterPartNum.y - 1 + out_index.y)*imposterPartNum.x + out_index.x]);
#else
	out_offsetPos = polyDir * (imposterParameter.x * ImposterSizeToCamera[(imposterPartNum.y - 1 + out_index.y)*imposterPartNum.x + out_index.x]);
#endif

	//Y軸回転オフセット
#if defined(INSTANCING)
	YRot -= InstancingImposterParam[instanceID].y;
#else
	YRot -= imposterParameter.y;
#endif
	out_index.x = (int)round(-YRot / PI2 * imposterPartNum.x) + (int)(imposterPartNum.x / 2.0f - 0.5f);

}

//頂点シェーダ(通常)
PSInput VSMain_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	//インポスター情報の計算
	int2 index;
	float4x4 rotMat;
	float3 offsetPos, offsetPosOld;
	CalcImposter(index, rotMat, offsetPos
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//座標合成
	offsetPosOld = offsetPos;
#if defined(INSTANCING)
	offsetPos += float3(InstancingWorldMatrix[instanceID]._m03, InstancingWorldMatrix[instanceID]._m13, InstancingWorldMatrix[instanceID]._m23);
	offsetPosOld += float3(InstancingWorldMatrixOld[instanceID]._m03, InstancingWorldMatrixOld[instanceID]._m13, InstancingWorldMatrixOld[instanceID]._m23);
#else
	offsetPos += float3(mWorld._m03, mWorld._m13, mWorld._m23);
	offsetPosOld += float3(mWorld_old._m03, mWorld_old._m13, mWorld_old._m23);
#endif

	//行列合成
	float4x4 worldMat, worldMatOld;
#if defined(INSTANCING)
	worldMat = mul(rotMat, InstancingWorldMatrix[instanceID]);
	worldMatOld = mul(rotMat, InstancingWorldMatrixOld[instanceID]);
#else
	worldMat = mul(rotMat, mWorld);
	worldMatOld = mul(rotMat, mWorld_old);	
#endif
	worldMat._m03 = offsetPos.x; worldMat._m13 = offsetPos.y; worldMat._m23 = offsetPos.z;
	worldMatOld._m03 = offsetPosOld.x; worldMatOld._m13 = offsetPosOld.y; worldMatOld._m23 = offsetPosOld.z;
	
	//通常処理
	PSInput psInput = VSModel(In, worldMat, worldMatOld
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//インデックス設定
	psInput.imposterIndex = index;
	
	return psInput;
}
//頂点シェーダ(深度値)
ZPSInput VSMain_RenderZ_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {	
	//インポスター情報の計算
	int2 index;
	float4x4 worldMat;
	float3 pos;
	CalcImposter(index, worldMat, pos
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//行列合成
#if defined(INSTANCING)
	worldMat = mul(worldMat, InstancingWorldMatrix[instanceID]);
#else
	worldMat = mul(worldMat, mWorld);
#endif

	//座標合成
#if defined(INSTANCING)
	worldMat._m03 = InstancingWorldMatrix[instanceID]._m03; worldMat._m13 = InstancingWorldMatrix[instanceID]._m13; worldMat._m23 = InstancingWorldMatrix[instanceID]._m23;
#else
	worldMat._m03 = mWorld._m03; worldMat._m13 = mWorld._m13; worldMat._m23 = mWorld._m23;
#endif

	//通常処理
	ZPSInput psInput = VSModel_RenderZ(In, worldMat
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//インデックス設定
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
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;

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

    TranslucentRender(In, Out);

	return Out;
}

#if defined(TEXTURE)
//インポスターのZ値出力
float4 PSMain_ImposterRenderZ(ZPSInput In) : SV_Target0
{
	//インデックスからuv座標を算出
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;

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

#endif //DW_IMPOSTER_FX