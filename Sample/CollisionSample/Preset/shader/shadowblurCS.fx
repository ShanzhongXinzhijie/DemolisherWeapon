
//定数
cbuffer CSCb : register(b0) {
	uint win_x;
	uint win_y;
};

// 入力テクスチャ
Texture2D<float4> shadowMap : register(t1);
// 出力テクスチャ
RWTexture2D<float4> rwOutputTex : register(u0);

static const float samples = 8.0f;

[numthreads(32, 32, 1)]
void CSmain(uint3 run_xy : SV_DispatchThreadID){

	uint2 uv = run_xy.xy;

	/*if (uv.x % 2 == 0) {
		uv.x = win_x - uv.x;
	}
	if (uv.y % 2 == 0) {
		uv.y = win_y - uv.y;
	}*/

	//テクスチャサイズ超えてたら出る
	if (uv.x > win_x || uv.y > win_y) {
		return;
	}

	//Z値取得
	float z_smap = shadowMap[uv].y;

	//何もなければ何もしない
	if (z_smap < 0.0f) {
		return;
	}

	float loopmax = samples;// min(samples, max(abs(velocity.x), abs(velocity.y)) / 2.5f);

	[unroll]
	for (float i = 0.0f; i < loopmax; i++)
	{
		uint2 sampuv = round((float2)uv + ((i + 1.0f) / loopmax) * velocity.xy);

		if (sampuv.x < 0 || sampuv.y < 0 || sampuv.x > win_x || sampuv.y > win_y) { break; }

		float4 samp = rwVelocityTex[sampuv];

		if (velocity.z < samp.w + 15.0f) {//手前のピクセルからはサンプルしない
			rwOutputTex[sampuv] = lerp(rwOutputTex[sampuv], color, 1.0f / (i + 2.0f));//サンプル数で透明度
		}
	}
}
