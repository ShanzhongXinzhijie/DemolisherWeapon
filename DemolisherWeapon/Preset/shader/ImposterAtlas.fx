Texture2D<float4> AlbedoTex : register(t0);
Texture2D<float3> NormalTex : register(t1);
Texture2D<float4> LightingTex : register(t2);

sampler Sampler : register(s0);

struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	Out.pos = In.pos;
	Out.uv = In.uv;
	return Out;
}

struct PSOutput {
	float4 albedo		: SV_Target0;		//アルベド
	float3 normal		: SV_Target1;		//法線
	float4 lightingParam: SV_Target2;		//ライティング用パラメーター
};
PSOutput PSMain(PSInput In)
{
	PSOutput Out = (PSOutput)0;

	In.uv -= float2(0.0003f, 0.0003f);
	for (uint x = 0; x < 3; x++) {
		for (uint y = 0; y < 3; y++) {
			Out.albedo += AlbedoTex.Sample(Sampler, In.uv);
			Out.normal += NormalTex.Sample(Sampler, In.uv);
			Out.lightingParam += LightingTex.Sample(Sampler, In.uv);
			In.uv.x += 0.0003f;
		}
		In.uv.x += 0.0003f;
	}

	Out.albedo /= 9.0f;
	Out.normal /= 9.0f;
	Out.lightingParam /= 9.0f;

	return Out;
}