#include"model.fx"

#if defined(INSTANCING)
//インスタンシング用テクスチャインデックス
StructuredBuffer<int2> InstancingImposterTextureIndex : register(t7);
#endif

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
	In.TexCoord.x += (1.0f / imposterPartNum.x) * InstancingImposterTextureIndex[In.instanceID].x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * InstancingImposterTextureIndex[In.instanceID].y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * imposterIndex.y;
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
	In.TexCoord.x += (1.0f / imposterPartNum.x) * InstancingImposterTextureIndex[In.instanceID].x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * InstancingImposterTextureIndex[In.instanceID].y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * imposterIndex.y;
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