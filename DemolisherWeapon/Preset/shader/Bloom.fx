//定数
cbuffer CSCb : register(b0) {
	//フレームバッファ解像度
	uint win_x;
	uint win_y;
	//出力テクスチャ解像度
	uint out_x;
	uint out_y;

	//輝度しきい値
	float luminanceThreshold;
};

// 入力テクスチャ
Texture2D<float4> inputTex : register(t1);
Texture2D<float4> interferenceFringes : register(t2);//干渉縞

// 出力テクスチャ
RWTexture2D<float4> rwOutputTex : register(u0);

//サンプラ
sampler Sampler : register(s0);

//グレア方向
static const float2 dirs[4] = {
	float2(4.0f, 4.0f),
	float2(3.0f, 0.0f),
	float2(0.0f, 3.0f),
	float2(-2.5f, 4.0f),
};
//ブラー倍率
static const float blurScale[12] = { 2.0f, 1.0f, 0.0f, 4.0f, 2.0f, 0.0f, 3.0f, 1.0f, 2.0f, 0.0f, 4.0f, 1.0f };

[numthreads(32, 32, 1)]
void CSmain(uint3 run_xy : SV_DispatchThreadID)
{
	uint2 uv = run_xy.xy;

	uv.x *= (float)win_x / out_x;
	uv.y *= (float)win_y / out_y;

	/*if (uv.x % 2 == 0) {
		uv.x = win_x - uv.x;
	}
	if (uv.y % 2 == 0) {
		uv.y = win_y - uv.y;
	}*/

	//テクスチャサイズ超えてたら出る
	if (uv.x > win_x || uv.y > win_y || run_xy.x > out_x || run_xy.y > out_y) {
		return;
	}			
	
	//色取得
	float4 color = inputTex[uv];

	//輝度が設定以下なら抜ける
	float luminance = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f));
	if (luminance < luminanceThreshold){
		return;
	}

	//出力
	color.xyz -= color.xyz * (luminanceThreshold / luminance);
	color.w = 1.0f;
	rwOutputTex[run_xy.xy] = lerp(color, rwOutputTex[run_xy.xy], 0.5f);

	uint f = 1280.0f / out_x * run_xy.x + 720.0f / out_y * run_xy.y;
	//グレアの大きさを座標に応じて変える
	luminance *= blurScale[f % 12]*2.0f;
	//色を座標に応じて変える
	color *= interferenceFringes.SampleLevel(Sampler, float2(0.05f * (f % 21), 0.5f), 0);
	
	//輝度が高いほどループ回数増える
	float loopmax = min(12.0f, luminance / luminanceThreshold);

	//解像度補正*輝度補正
	float2 sizeScale = float2((float)win_x / 1280.0f, (float)win_y / 720.0f) * (luminance / luminanceThreshold);

	//win→outへの変換倍率
	float2 outScale = float2((float)out_x / win_x, (float)out_y / win_y);

	//グレア描画
	[unroll]
	for (uint i2 = 0; i2 < 4; i2++)
	{
		[unroll]
		for (float i = 0.0f; i < loopmax; i++)
		{
			uint2 sampuv = round((float2)uv + ((i + 1.0f) / loopmax) * dirs[i2] * sizeScale);
			uint2 sampuv2 = round((float2)uv + (-((i + 1.0f) / loopmax)) * dirs[i2] * sizeScale);	
			sampuv *= outScale;
			sampuv2 *= outScale;

			if (sampuv.x >= 0 && sampuv.y >= 0 && sampuv.x <= out_x && sampuv.y <= out_y) {
				rwOutputTex[sampuv] = lerp(color, rwOutputTex[sampuv], 0.5f);
			}
			if (sampuv2.x >= 0 && sampuv2.y >= 0 && sampuv2.x <= out_x && sampuv2.y <= out_y) {
				rwOutputTex[sampuv2] = lerp(color, rwOutputTex[sampuv2], 0.5f);
			}
		}
	}
}