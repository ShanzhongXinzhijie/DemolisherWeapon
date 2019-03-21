//定数
cbuffer CSCb : register(b0) {
	//フレームバッファ解像度
	uint win_x;
	uint win_y;
	//距離スケール
	float distanceScale;
};

// 入力テクスチャ
Texture2D<float4> rwVelocityTex : register(t1);
Texture2D<float4> albedoTexture : register(t2);

// 出力テクスチャ
RWTexture2D<float4> rwOutputTex		: register(u0);
//RWTexture2D<float4> rwVelocityTex	: register(u1);

#include"MotionBlurHeader.h"

[numthreads(32, 32, 1)]
void CSmain(uint3 run_xy : SV_DispatchThreadID)
{
	uint2 uv = run_xy.xy;

	if (uv.x % 2 == 0) {
		uv.x = win_x - uv.x;
	}
	if (uv.y % 2 == 0) {
		uv.y = win_y - uv.y;
	}

	//テクスチャサイズ超えてたら出る
	if (uv.x > win_x || uv.y > win_y) {
		return;
	}			

	//速度取得
	float3 velocity = rwVelocityTex[uv].xyz;
	velocity.xy *= blurscale;	

	//速度小さいと何もしない
	if (abs(velocity.x) < BUNBO*0.5f && abs(velocity.y) < BUNBO*0.5f) {//0.001f
		return;
	}
			
	//色取得
	float4 color = albedoTexture[uv];

	float loopmax = min(samples,max(abs(velocity.x), abs(velocity.y))/BUNBO);

	//座標変換
	velocity.x *= (float)win_x;
	velocity.y *= (float)win_y;

	[unroll]
	for (float i = 0.0f; i < loopmax; i++)
	{
		uint2 sampuv = round((float2)uv + ((i + 1.0f) / loopmax) * velocity.xy);

		if (sampuv.x < 0 || sampuv.y < 0 || sampuv.x > win_x || sampuv.y > win_y) { break; }
		
		float4 samp = rwVelocityTex[sampuv];

		if (velocity.z < samp.w+Z_OFFSET*distanceScale) {//手前のピクセルからはサンプルしない
			rwOutputTex[sampuv] = lerp(rwOutputTex[sampuv], color, 1.0f / (i + 2.0f));//サンプル数で透明度
		}
	}
}

//
[numthreads(32, 32, 1)]
void CSresolve(uint3 run_xy : SV_DispatchThreadID)
{
	uint2 uv = run_xy.xy;

	//テクスチャサイズ超えてたら出る
	if (uv.x > win_x || uv.y > win_y) {
		return;
	}

	float sampcnt = rwVelocityTex[uv].w;
	if (sampcnt > 0.0f) {
		rwOutputTex[uv] /= (sampcnt + 1.0f);
	}
}