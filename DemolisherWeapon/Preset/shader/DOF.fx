struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

Texture2D<float4> FRT : register(t0);			//最終レンダーターゲット
Texture2D<float4> ResizeBuffer : register(t1);	//縮小バッファ
Texture2D<float4> PosMap : register(t2);		//座標マップ(wの深度値を使用)
sampler Sampler : register(s0);

//定数バッファ
//[DepthOfFieldRender.h : SPSConstantBuffer]
cbuffer PSCb : register(b0) {
	float focus;
	float focusArea;
	float Near;
	float Far;
};

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = In.pos;
	psIn.uv = In.uv;
	return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
	float4 Resize = ResizeBuffer.Sample(Sampler, In.uv);
	float4 Normal = FRT.Sample(Sampler, In.uv);
    float depth = length(PosMap.Sample(Sampler, In.uv).xyz);
	
	float fade = 0.0f;
	if (depth > focus + focusArea) {
		fade = abs(depth - (focus + focusArea)) / abs(Far - (focus + focusArea));
	}
	else if (depth < focus - focusArea) {
		fade = abs((focus - focusArea) - depth) / abs((focus - focusArea) - Near);
	}
	fade = saturate(fade);

	//ノイズテスト
	//Interleaved gradient noise
	/*float3 magic = float3(0.06711056, 0.00583715, 52.9829189);
	float noise = frac(magic.z * frac(dot(In.pos, magic.xy)));
	return float4(noise, noise, noise, 1.0f);*/

	//return float4(fade, 0.0f, 1.0f, 1.0f);
	return lerp(Normal, Resize, fade);
}