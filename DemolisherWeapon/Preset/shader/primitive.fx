Texture2D<float4> Texture : register(t0);

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

float4 PSMain(PSInput In) : SV_Target0
{
	return Texture.Sample(Sampler, In.uv);
}

float4 PSConvertLinearToSRGB(PSInput In) : SV_Target0
{
	float4 Out = Texture.Sample(Sampler, In.uv);
	Out.xyz = pow(Out.xyz, 1.0f / 2.2f);
	return Out;
}