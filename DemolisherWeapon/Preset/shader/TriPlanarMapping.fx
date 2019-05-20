#include"model.fx"

PSOutput_RenderGBuffer PS_TriPlanarMapping(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	float3 uv = In.Worldpos * triPlanarMapUVScale;
	float3 blendNormal = saturate(pow(In.Normal*1.4f, 4));

	//アルベド
#if ALBEDO_MAP
	float4 X = albedoTexture.Sample(Sampler, uv.zy);
	float4 Y = albedoTexture.Sample(Sampler, uv.zx);
	float4 Z = albedoTexture.Sample(Sampler, uv.xy);

	Out.albedo = Z;
	Out.albedo = lerp(Out.albedo, X, blendNormal.x);
	Out.albedo = lerp(Out.albedo, Y, blendNormal.y);	
#else
	AlbedoRender(In, Out);
#endif

	//αテスト
	if (Out.albedo.a > 0.5f) {
		Out.albedo.a = 1.0f;//半透明無効
	}
	else {
		discard;//描画しない
	}

	//法線
	//多分だめ
	//https://www.patreon.com/posts/16714688
	//https://medium.com/@bgolus/normal-mapping-for-a-triplanar-shader-10bf39dca05a	
#if NORMAL_MAP
	float3 nX = NormalTexture.Sample(Sampler, uv.zy);
	float3 nY = NormalTexture.Sample(Sampler, uv.zx);
	float3 nZ = NormalTexture.Sample(Sampler, uv.xy);

	Out.normal = nZ;
	Out.normal = lerp(Out.normal, nX, blendNormal.x);
	Out.normal = lerp(Out.normal, nY, blendNormal.y);

	Out.normal = Out.normal * 2.0f - 1.0f;
	Out.normal = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal = normalize(Out.normal);
#else
	Out.normal = In.Normal;
#endif

	//その他出力
	PosRender(In, Out);
	ParamRender(In, Out);
	MotionRender(In, Out);

	return Out;
}