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
	//color.xyz *= luminance - luminanceThreshold;
	color.xyz -= color.xyz * (luminanceThreshold / luminance);
	color.w = 1.0f;
	rwOutputTex[uv] = lerp(color, rwOutputTex[uv], 0.5f);

	//輝度が高いほどループ回数増える
	float loopmax = min(6.0f, luminance / luminanceThreshold);

	//解像度補正*輝度補正
	float2 sizeScale = float2(win_x / 1280.0f, win_y / 720.0f) * (luminance / luminanceThreshold);

	[unroll]
	for (uint i2 = 0; i2 < 4; i2++)
	{
		[unroll]
		for (float i = 0.0f; i < loopmax; i++)
		{
			uint2 sampuv = round((float2)uv + ((i + 1.0f) / loopmax) * dirs[i2] * sizeScale);
			uint2 sampuv2 = round((float2)uv + (-((i + 1.0f) / loopmax)) * dirs[i2] * sizeScale);			

			uint breakCnt = 0;
			float4 outColor = color * interferenceFringes.SampleLevel(Sampler, float2(i / 6.0f, 0.5f), 0);
			if (sampuv.x < 0 || sampuv.y < 0 || sampuv.x > win_x || sampuv.y > win_y) {
				breakCnt++;
			}
			else {
				rwOutputTex[sampuv] = lerp(outColor, rwOutputTex[sampuv], 0.5f);
			}
			if (sampuv2.x < 0 || sampuv2.y < 0 || sampuv2.x > win_x || sampuv2.y > win_y) {
				breakCnt++;
			}
			else {
				rwOutputTex[sampuv2] = lerp(outColor, rwOutputTex[sampuv2], 0.5f);
			}
			if (breakCnt == 2) {
				break;
			}

			//rwOutputTex[sampuv] = lerp(rwOutputTex[sampuv], color, 1.0f / (i + 2.0f));//サンプル数で透明度		
			//rwOutputTex[sampuv2] = lerp(rwOutputTex[sampuv2], color, 1.0f / (i + 2.0f));//サンプル数で透明度
			//rwOutputTex[sampuv] += color / (i + 2.0f);//サンプル数で透明度		
			//rwOutputTex[sampuv2] += color / (i + 2.0f);//サンプル数で透明度			
		}
	}
}