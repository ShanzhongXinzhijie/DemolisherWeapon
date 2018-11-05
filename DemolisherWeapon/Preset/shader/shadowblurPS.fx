
struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

Texture2D<float> shadowMap : register(t0);

sampler Sampler : register(s0);

static const uint samples = 8;

cbuffer CBBlur : register(b0){
	float weight[samples];//!<重み。
};

PSInput VSBlur(VSInput In){
	PSInput Out;
	Out.pos = In.pos;
	Out.uv = In.uv;
	return Out;
}

float Blur(float2 uv, float2 offset[samples], float motoColor)
{
	float2 Color = float2(0.0f, 0.0f);

	for (uint i = 0; i < samples; i++) {
		/*float4 addcolor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
		float4 color = shadowMap.Sample(Sampler, uv + offset[i]);
		if (color.x > motoColor.x) {
			color = motoColor;
		}
		addcolor += color;

		color = shadowMap.Sample(Sampler, uv - offset[i]);
		if (color.x > motoColor.x) {
			color = motoColor;
		}
		addcolor += color;*/

		float color1 = shadowMap.Sample(Sampler, uv + offset[i]);
		float color2 = shadowMap.Sample(Sampler, uv - offset[i]);
		
		//Color.x = min(motoColor.x,min(color1.x, color2.x));
		Color.x += weight[i] * (color1 + color2);//addcolor;
	}

	return Color.x;
}

/*!
 * @brief	Xブラーピクセルシェーダー。
 */
float4 PSXBlur(PSInput In) : SV_Target0
{
	//Z値取得
	float z = shadowMap.Sample(Sampler, In.uv);

	//z値なければ実行
	if (1){//z.x > 1.0f) {

		//X軸方向に調べる。
		float2 offset[samples];// = float2(0.0f, 0.0f);
		for (int i = 0; i < samples; i++) {
			if (i == 0) {
				offset[i].x = 10.0f / 1280.0f;
			}
			else {
				offset[i].x = offset[i - 1].x + 10.0f / 1280.0f;
			}
			offset[i].y = 0.0f;
		}

		return Blur(In.uv, offset, z);// float4(Blur(In.uv, offset, z), 0.0f, 0.0f);
	}
	else {
		return z;
	}
}

/*!
 * @brief	Yブラーピクセルシェーダー。
 */
float4 PSYBlur(PSInput In) : SV_Target0
{
	//Z値取得
	float z = shadowMap.Sample(Sampler, In.uv);

	//z値なければ実行
	if (1) {//z.x > 1.0f) {

		//Y軸方向に調べる。
		float2 offset[samples];// = float2(0.0f, 0.0f);
		for (int i = 0; i < samples; i++) {
			offset[i].x = 0.0f;
			if (i == 0) {
				offset[i].y = 10.0f / 720.0f;
			}
			else {
				offset[i].y = offset[i - 1].y + 10.0f / 720.0f;
			}
		}

		return Blur(In.uv, offset, z);// float4(Blur(In.uv, offset, z), 0.0f, 0.0f);
	}
	else {
		return z;
	}
}