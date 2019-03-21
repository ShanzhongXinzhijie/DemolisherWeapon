//定数
cbuffer PSCb : register(b0) {
	//ビュープロジェクション逆行列
	float4x4 ViewProjInv;
	//距離スケール
	float distanceScale;
};

// 入力テクスチャ
Texture2D<float4> normalTexture : register(t1);
Texture2D<float4> posTexture : register(t2);

sampler Sampler : register(s0);

static const float AO_RANGE = 5000.0f;
static const float AO_FAR_RANGE = 22.36f;// 06797749979f;// 500.0f;

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

struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PSInput VSmain(VSInput In)
{
	PSInput Out;
	Out.pos = In.pos;
	Out.uv = In.uv;
	return Out;
}

float PSmain(PSInput In) : SV_Target0
{
	float3 normal = normalTexture.Sample(Sampler, In.uv).xyz;
	float4 viewpos = posTexture.Sample(Sampler, In.uv);
	float3 worldpos = CalcWorldPosFromUVZ(In.uv, viewpos.w, ViewProjInv);

	int maxcnt = 7 * saturate(1.0f - viewpos.z / (AO_RANGE*distanceScale)) + 0.5f;
	float ao = 0.0f;
	[unroll]
	for (int i2 = 1; i2 < maxcnt; i2++) {
		[unroll]
		for (int i = 0; i < 2; i++) {
			float2 uv2 = In.uv, uv22 = In.uv;
			const float offset   = 0.0015625f;// 1.0f + 3.0f*((float)i2 / (maxcnt - 1.0f));// +1.0f*(maxcnt / 7.0f);
			const float offset_y = 0.0027778f; 
			if (i2 % 2 != 0) {
				if (i == 0) {
					uv2.x -= offset * i2; uv22.x += offset * i2;
					uv2.y -= offset_y * i2; uv22.y += offset_y * i2;
				}
				if (i == 1) {
					uv2.x -= offset * i2; uv22.x += offset * i2;
					uv2.y += offset_y * i2; uv22.y -= offset_y * i2;
				}
			}
			else {
				if (i == 0) {
					uv2.x -= offset * i2; uv22.x += offset * i2;
				}
				if (i == 1) {
					uv2.y -= offset_y * i2; uv22.y += offset_y * i2;
				}
			}

			if (uv2.x  < 0.0f || uv2.y  < 0.0f || uv2.x  > 1.0f || uv2.y  > 1.0f) { continue; }
			if (uv22.x < 0.0f || uv22.y < 0.0f || uv22.x > 1.0f || uv22.y > 1.0f) { continue; }

			float4 viewpos2 = posTexture.Sample(Sampler, uv2);
			float4 viewpos22 = posTexture.Sample(Sampler, uv22);

			if (viewpos.z - viewpos2.z > 0.0f && viewpos.z - viewpos22.z > 0.0f) {
			
			float3 vec;
			vec = viewpos.xyz - viewpos2.xyz;
			float dis = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
			vec = viewpos.xyz - viewpos22.xyz;
			float dis2 = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;

			float rangeSqr = AO_FAR_RANGE * distanceScale; rangeSqr *= rangeSqr;
			if (dis < rangeSqr && dis2 < rangeSqr) {
				float3 worldpos2 = CalcWorldPosFromUVZ(uv2, viewpos2.w, ViewProjInv);
				float3 worldpos22 = CalcWorldPosFromUVZ(uv22, viewpos22.w, ViewProjInv);
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
	return saturate(ao);
}