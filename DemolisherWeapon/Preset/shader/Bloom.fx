//定数
cbuffer CSCb : register(b0) {
	//フレームバッファ解像度
	uint win_x;
	uint win_y;

	//輝度しきい値
	float luminanceThreshold;
};

// 入力テクスチャ
Texture2D<float4> inputTex : register(t1);

// 出力テクスチャ
RWTexture2D<float4> rwOutputTex : register(u0);

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
	
	//色取得
	float4 color = inputTex[uv];

	//輝度が設定以下なら抜ける
	float luminance = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f));
	if (luminance < luminanceThreshold){
		return;
	}

	//出力
	color.xyz *= luminance - luminanceThreshold;
	color.w = 1.0f;
	rwOutputTex[uv] = color;

	//輝度が高いほどループ回数増える
	//float loopmax = min(12.0f, luminance / 2.0f);

	//座標変換
	/*velocity.x *= (float)win_x;
	velocity.y *= (float)win_y;*/

	//[unroll]
	//for (uint i2 = 0; i2 < loopmax; i2++)
	//{
	//	[unroll]
	//	for (float i = 0.0f; i < loopmax; i++)
	//	{
	//		uint2 sampuv = round((float2)uv + ((i + 1.0f) / loopmax) * velocity.xy);

	//		if (sampuv.x < 0 || sampuv.y < 0 || sampuv.x > win_x || sampuv.y > win_y) { break; }

	//		float4 samp = rwVelocityTex[sampuv];

	//		if (velocity.z < samp.w + Z_OFFSET * distanceScale) {//手前のピクセルからはサンプルしない
	//			rwOutputTex[sampuv] = lerp(rwOutputTex[sampuv], color, 1.0f / (i + 2.0f));//サンプル数で透明度
	//		}
	//	}
	//}
}