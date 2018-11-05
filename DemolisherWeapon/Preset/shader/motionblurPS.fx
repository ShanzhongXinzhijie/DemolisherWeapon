struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};

struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

Texture2D<float4> sceneTexture : register(t0);	//シーンテクスチャ
Texture2D<float4> VelocityMap : register(t1);	//ベロシティマップ
sampler Sampler : register(s0);

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = In.pos;
	psIn.uv = In.uv;
	return psIn;
}

static const float samples = 8;
static const float blurscale = -0.15f;// 0.4f;
static const float BUNBO = 0.002f*(8.0f / samples);//0.001953125f

float4 PSMain(PSInput In) : SV_Target0
{
	float4 Out = sceneTexture.Sample(Sampler, In.uv);
	
	//ベロシティマップ取得
	float3 velocity = VelocityMap.Sample(Sampler, In.uv);
	velocity.xy *= blurscale;

	if (abs(velocity.x) < BUNBO*0.5f && abs(velocity.y) < BUNBO*0.5f) {//0.001f
		return Out;
	}

	float loopmax = min(samples, max(abs(velocity.x), abs(velocity.y)) / BUNBO);

	//　サンプリング
	float samplecnt = 0.0f;
	for (float i = 0; i < loopmax; i++)
	{
		float t = (i + 1) / loopmax;
		float sampz = VelocityMap.Sample(Sampler, In.uv + t * velocity).w;
		if (sampz > 0.0f){// && velocity.z < sampz + Z_OFFSET) {//手前のピクセルからはサンプルしない
			Out += sceneTexture.Sample(Sampler, In.uv + t * velocity);
			samplecnt += 1.0f;
		}
	}

	if (samplecnt < 0.9f) {
		return Out;
	}
	else {
		//　平均を求める
		Out /= samplecnt + 1;
	}

	return Out;
}