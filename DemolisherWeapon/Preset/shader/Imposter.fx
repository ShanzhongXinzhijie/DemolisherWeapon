#include"model.fx"

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
	finalOut.normal = Out.normal; finalOut.normal.xy *= -1.0f;
	finalOut.lightingParam.x = Out.lightingParam.x;	//エミッシブ
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
	
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;

	In.TexCoord.x += (1.0f / imposterPartNum.x) * imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * imposterIndex.y;

#if ALBEDO_MAP && NORMAL_MAP
	if(Out.albedo.a < 0.1f)
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);
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

#if ALBEDO_MAP && NORMAL_MAP
	Out.normal = NormalTexture.Sample(Sampler, In.TexCoord + uvOffset);
	//Out.normal = Out.normal * 2.0f - 1.0f;
	Out.normal = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal = normalize(Out.normal);
#endif

	PosRender(In, Out);//これでいいのか

	ParamRender(In, Out);

	MotionRender(In, Out);

	return Out;
}