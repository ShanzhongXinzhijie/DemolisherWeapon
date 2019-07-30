//定数
cbuffer CSCb : register(b0) {
	//フレームバッファ解像度
	uint win_x;
	uint win_y;
	//AO出力テクスチャ解像度
	uint ao_x;
	uint ao_y;
	//ビュープロジェクション逆行列
	float4x4 ViewProjInv;
	//距離スケール
	float distanceScale;
};

// 入力テクスチャ
Texture2D<float4> normalTexture : register(t1);
Texture2D<float4> posTexture : register(t2);

// 出力テクスチャ
RWTexture2D<float> rwOutputTex		: register(u0);

static const float AO_RANGE = 5000.0f;
static const float AO_FAR_RANGE = 22.36f;// 06797749979f;// 500.0f;
//static const uint offset = 2;// 1.0f + 3.0f*((float)i2 / (maxcnt - 1.0f));// +1.0f*(maxcnt / 7.0f);
static const int2 offset_2[4] = {
	int2(-2,-2),
	int2(-2, 2),
	int2(-2, 0),
	int2(0,-2),
};
static const int2 offset_22[4] = {
	int2(2, 2),
	int2(2,-2),
	int2(2, 0),
	int2(0, 2),
};

//Z値からワールド座標を復元
float3 CalcWorldPosFromUVZ(float2 uv, float zInScreen, float4x4 mViewProjInv)
{
	float3 screenPos;
	screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
	screenPos.z = zInScreen;

	float4 worldPos = mul(mViewProjInv, float4(screenPos, 1.0f));
	worldPos.xyz /= worldPos.w;
	return worldPos.xyz;
}

[numthreads(32, 32, 1)]
void CSmain(uint3 run_xy : SV_DispatchThreadID)
{
	uint2 uv = run_xy.xy;
	//テクスチャサイズ超えてたら出る
	if (uv.x > ao_x || uv.y > ao_y) {
		return;
	}

	uint2 sampuv = uint2(uv.x *((float)win_x / ao_x)+0.5f, uv.y *((float)win_y / ao_y)+0.5f);
	float3 normal = normalTexture[sampuv].xyz;
	float4 viewpos = posTexture[sampuv];
	float3 worldpos = CalcWorldPosFromUVZ(float2(sampuv.x/ (float)win_x, sampuv.y / (float)win_y), viewpos.w, ViewProjInv);

	int maxcnt = 7 * saturate(1.0f - viewpos.z / (AO_RANGE*distanceScale)) + 0.5f;
	float ao = 0.0f;
	float2 uvScale = float2(win_x / 1280.0f, win_y / 720.0f);
	[unroll]
	for (int i2 = 1; i2 < maxcnt; i2++) {
		[unroll]
		for (int i = 0; i < 2; i++) {
			uint2 uv2 = sampuv, uv22 = sampuv;

			uv2  += offset_2[i + 2 * (i2 % 2)] * i2 * uvScale;
			uv22 += offset_22[i + 2 * (i2 % 2)] * i2 * uvScale;
			
			if (uv2.x  < 0 || uv2.y  < 0 || uv2.x  > win_x || uv2.y  > win_y) { continue; }
			if (uv22.x < 0 || uv22.y < 0 || uv22.x > win_x || uv22.y > win_y) { continue; }

			float4 viewpos2 = posTexture[uv2];
			float4 viewpos22 = posTexture[uv22];

			if (viewpos.z - viewpos2.z > 0.0f && viewpos.z - viewpos22.z > 0.0f) {
			
			float3 vec;
			vec = viewpos.xyz - viewpos2.xyz;
			float dis = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
			vec = viewpos.xyz - viewpos22.xyz;
			float dis2 = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;

			float rangeSqr = AO_FAR_RANGE * distanceScale; rangeSqr *= rangeSqr;
			if (dis < rangeSqr && dis2 < rangeSqr) {
				float3 worldpos2 = CalcWorldPosFromUVZ(float2(uv2.x / (float)win_x, uv2.y / (float)win_y), viewpos2.w, ViewProjInv);
				float3 worldpos22 = CalcWorldPosFromUVZ(float2(uv22.x / (float)win_x, uv22.y / (float)win_y), viewpos22.w, ViewProjInv);
				float3 normal2 = normalize(worldpos2.xyz - worldpos.xyz);
				float3 normal22 = normalize(worldpos22.xyz - worldpos.xyz);

				//float fdot = dot(normal, normal2)*dot(normal, normal22);
				//saturate(1.0f - fdot) * 

				ao += saturate(1.0f - abs(acos(dot(normal2, normal22))) / 3.14f)*0.25f*(1.0f- dis / rangeSqr)*(1.0f - dis2 / rangeSqr);// *saturate(viewpos.z - viewpos2.z)*saturate(viewpos.z - viewpos22.z);
			}

			}
		}
	}
	ao = 1.0f - ao;
	rwOutputTex[uv] = saturate(ao);

}