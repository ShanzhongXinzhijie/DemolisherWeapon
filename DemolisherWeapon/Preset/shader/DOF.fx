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

	//カメラ設定
	float camNear;
	float camFar;
};

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = In.pos;
	psIn.uv = In.uv;
	return psIn;
}

//デプス値を線形に変換
float LinearizeDepth(float depth)
{
	return (2.0 * camNear) / (camFar + camNear - depth * (camFar - camNear));
}

float4 PSMain(PSInput In) : SV_Target0
{
	float4 Resize = ResizeBuffer.Sample(Sampler, In.uv);
	float4 Normal = FRT.Sample(Sampler, In.uv);
	float depth = LinearizeDepth(PosMap.Sample(Sampler, In.uv).w);
	
	float fade = 0.0f;
	if (depth > focus + focusArea) {
		fade = abs(depth - (focus + focusArea)) / abs(Far - (focus + focusArea));
	}
	else if (depth < focus - focusArea) {
		fade = abs((focus - focusArea) - depth) / abs((focus - focusArea) - Near);
	}
	fade = saturate(fade);

	//return float4(fade, 0.0f, 1.0f, 1.0f);
	return lerp(Normal, Resize, fade);
}