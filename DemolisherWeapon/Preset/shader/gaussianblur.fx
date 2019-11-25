//ぼかすテクスチャ
Texture2D<float4> blurTexture : register(t0);

//入力
struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PS_BlurInput
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float2 tex1 : TEXCOORD1;
    float2 tex2 : TEXCOORD2;
    float2 tex3 : TEXCOORD3;
    float2 tex4 : TEXCOORD4;
    float2 tex5 : TEXCOORD5;
    float2 tex6 : TEXCOORD6;
    float2 tex7 : TEXCOORD7;
};
//サンプラー
sampler Sampler : register(s0);

//定数バッファ
//[GaussianBlurRender.h : SBlurParam]
static const int NUM_WEIGHTS = 8;
cbuffer CBBlur : register(b0)
{
	float4 offset;					//オフセット(16.0f / 解像度)
	float4 weight[NUM_WEIGHTS/4];	//重み
    float sampleScale;				//サンプリング間隔
};

PS_BlurInput VSXBlur(VSInput In)
{
	PS_BlurInput Out;
    Out.pos = In.pos; //頂点座標出力

	//テクスチャサンプル座標算出
    float2 tex = In.uv;
    //tex += float2(-0.5f / sampleScale, 0.0f); //オフセット
    Out.tex0 = tex + float2(-1.0f / sampleScale, 0.0f);
    Out.tex1 = tex + float2(-3.0f / sampleScale, 0.0f);
    Out.tex2 = tex + float2(-5.0f / sampleScale, 0.0f);
    Out.tex3 = tex + float2(-7.0f / sampleScale, 0.0f);
    Out.tex4 = tex + float2(-9.0f / sampleScale, 0.0f);
    Out.tex5 = tex + float2(-11.0f / sampleScale, 0.0f);
    Out.tex6 = tex + float2(-13.0f / sampleScale, 0.0f);
    Out.tex7 = tex + float2(-15.0f / sampleScale, 0.0f);

    return Out;
}
PS_BlurInput VSYBlur(VSInput In)
{
    PS_BlurInput Out;
    Out.pos = In.pos; //頂点座標出力

	//テクスチャサンプル座標算出
    float2 tex = In.uv;
    //tex += float2(0.0f, -0.5f / sampleScale); //オフセット
	Out.tex0 = tex + float2(0.0f, -1.0f / sampleScale);
    Out.tex1 = tex + float2(0.0f, -3.0f / sampleScale);
    Out.tex2 = tex + float2(0.0f, -5.0f / sampleScale);
    Out.tex3 = tex + float2(0.0f, -7.0f / sampleScale);
    Out.tex4 = tex + float2(0.0f, -9.0f / sampleScale);
    Out.tex5 = tex + float2(0.0f, -11.0f / sampleScale);
    Out.tex6 = tex + float2(0.0f, -13.0f / sampleScale);
    Out.tex7 = tex + float2(0.0f, -15.0f / sampleScale);

    return Out;
}

//Xブラー頂点シェーダー
/*PS_BlurInput VSXBlur(VSInput In)
{
	//テクスチャ解像度取得
	float2 texSize;
	float level;
	blurTexture.GetDimensions(0, texSize.x, texSize.y, level);

	PS_BlurInput Out;
	Out.pos = In.pos;//頂点座標出力

	//テクスチャサンプル座標算出
	float2 tex = In.uv;
	//tex += float2(-0.5f / texSize.x, 0.0f); //オフセット
	Out.tex0 = tex + float2(-1.0f / texSize.x, 0.0f);
	Out.tex1 = tex + float2(-3.0f / texSize.x, 0.0f);
	Out.tex2 = tex + float2(-5.0f / texSize.x, 0.0f);
	Out.tex3 = tex + float2(-7.0f / texSize.x, 0.0f);
	Out.tex4 = tex + float2(-9.0f / texSize.x, 0.0f);
	Out.tex5 = tex + float2(-11.0f / texSize.x, 0.0f);
	Out.tex6 = tex + float2(-13.0f / texSize.x, 0.0f);
	Out.tex7 = tex + float2(-15.0f / texSize.x, 0.0f);

	return Out;
}

//Yブラー頂点シェーダー
PS_BlurInput VSYBlur(VSInput In)
{
	//テクスチャ解像度取得
    float2 texSize;
	float level;
	blurTexture.GetDimensions(0, texSize.x, texSize.y, level);

	PS_BlurInput Out;
    Out.pos = In.pos; //頂点座標出力

	//テクスチャサンプル座標算出
    float2 tex = In.uv;
    //tex += float2(0.0f, -0.5f / texSize.y); //オフセット
	Out.tex0 = tex + float2(0.0f, -1.0f / texSize.y);
	Out.tex1 = tex + float2(0.0f, -3.0f / texSize.y);
	Out.tex2 = tex + float2(0.0f, -5.0f / texSize.y);
	Out.tex3 = tex + float2(0.0f, -7.0f / texSize.y);
	Out.tex4 = tex + float2(0.0f, -9.0f / texSize.y);
	Out.tex5 = tex + float2(0.0f, -11.0f / texSize.y);
	Out.tex6 = tex + float2(0.0f, -13.0f / texSize.y);
	Out.tex7 = tex + float2(0.0f, -15.0f / texSize.y);

	return Out;
}*/

/*!
 * @brief	ブラーピクセルシェーダー。
 */
float4 PSBlur(PS_BlurInput In) : SV_Target0
{
	float4 Color;
	Color =  weight[0].x * (blurTexture.Sample(Sampler, In.tex0)
				   + blurTexture.Sample(Sampler, In.tex7 + offset.xy));
	Color += weight[0].y * (blurTexture.Sample(Sampler, In.tex1)
				   + blurTexture.Sample(Sampler, In.tex6 + offset.xy));
	Color += weight[0].z * (blurTexture.Sample(Sampler, In.tex2)
				   + blurTexture.Sample(Sampler, In.tex5 + offset.xy));
	Color += weight[0].w * (blurTexture.Sample(Sampler, In.tex3)
				   + blurTexture.Sample(Sampler, In.tex4 + offset.xy));
	Color += weight[1].x * (blurTexture.Sample(Sampler, In.tex4)
				   + blurTexture.Sample(Sampler, In.tex3 + offset.xy));
	Color += weight[1].y * (blurTexture.Sample(Sampler, In.tex5)
				   + blurTexture.Sample(Sampler, In.tex2 + offset.xy));
	Color += weight[1].z * (blurTexture.Sample(Sampler, In.tex6)
				   + blurTexture.Sample(Sampler, In.tex1 + offset.xy));
	Color += weight[1].w * (blurTexture.Sample(Sampler, In.tex7)
				   + blurTexture.Sample(Sampler, In.tex0 + offset.xy));
	return Color;
}
