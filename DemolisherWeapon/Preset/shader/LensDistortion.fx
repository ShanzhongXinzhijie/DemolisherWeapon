Texture2D<float4> Texture : register(t0);
Texture2D<float3> LutTexture : register(t1);

sampler Sampler : register(s0);

struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

//頂点シェーダ
PSInput VSMain(VSInput In)
{
	PSInput Out;
	Out.pos = In.pos;
	Out.uv = In.uv;
	return Out;
}

cbuffer PSCb : register(b0) {
	float k4;
	float LENS_DISTORTION_UV_MAGNIFICATION;
	float ASPECT_RATIO;
	float INV_ASPECT_RATIO;

    float2 resolution;
};

//ビネット(周辺減光)が始まる位置
static const float VIGNETTE_POS_MIN = 0.02f;
static const float VIGNETTE_POS_MAX = 1.0f - VIGNETTE_POS_MIN;
//終わる位置
static const float VIGNETTE_END_MIN = -0.006f;

/// @brief レンズ歪みを適用したUVを取得
//http://hikita12312.hatenablog.com/entry/2018/04/07/142219
//※ FinalRender::CalcLensDistortionと連動
float2 GetLensDistortionUV(in float2 uv)
{
	// Aspect比を考慮した空間上での,光軸中心からの距離ベクトル
	float2 rVec = (uv - float2(0.5f, 0.5f));
	rVec.x *= ASPECT_RATIO;
	const float r2 = dot(rVec, rVec);
	const float r4 = r2 * r2;
	const float k1 = 0.0f;
	const float k2 = 0.0f;
	const float k3 = 0.0f;

	// 半径方向歪みを考慮した距離ベクトル
	float2 distortionR = (1.0f + k1 * r2 + k2 * r4) / (1.0f + k3 * r2 + k4 * r4) * rVec;
	
	// 円周方向歪み量の加算
	/*const float p1 = 0.0f;
	const float p2 = 0.0f;
	const float C = 2.0f * rVec.x * rVec.y;
	const float2 V = 2.0f * rVec * rVec;
	distortionR += float2(p1*C + p2 * (r2 + V.x), p2*C + p1 * (r2 + V.y));*/

	// アスペクト比補正
	distortionR.x *= INV_ASPECT_RATIO;

	// 画面中心基準の拡大,UV座標への変換
	uv = (distortionR*LENS_DISTORTION_UV_MAGNIFICATION) + float2(0.5f, 0.5f);
	
	return uv;
}

//周辺減光
float4 Vignette(in float4 color, in float2 uv)
{
    if (uv.x < VIGNETTE_POS_MIN || uv.y < VIGNETTE_POS_MIN || uv.x > VIGNETTE_POS_MAX || uv.y > VIGNETTE_POS_MAX)
    {
        float2 sub = uv;
        if (uv.x < VIGNETTE_POS_MIN)
        {
            sub.x = VIGNETTE_POS_MIN;
        }
        else if (uv.x > VIGNETTE_POS_MAX)
        {
            sub.x = VIGNETTE_POS_MAX;
        }
        if (uv.y < VIGNETTE_POS_MIN)
        {
            sub.y = VIGNETTE_POS_MIN;
        }
        else if (uv.y > VIGNETTE_POS_MAX)
        {
            sub.y = VIGNETTE_POS_MAX;
        }

        uv -= float2(0.5f, 0.5f);
        uv.x *= ASPECT_RATIO;
        sub -= float2(0.5f, 0.5f);
        sub.x *= ASPECT_RATIO;

        return float4(lerp(color.xyz, float3(0.0f, 0.0f, 0.0f), distance(uv, sub) / distance(float2((VIGNETTE_END_MIN - 0.5f) * ASPECT_RATIO, VIGNETTE_END_MIN - 0.5f), float2((VIGNETTE_POS_MIN - 0.5f) * ASPECT_RATIO, VIGNETTE_POS_MIN - 0.5f))), color.w);
    }

    return color;
}

//FlipTri Antialiasing
//https://blog.demofox.org/2015/04/23/flipquad-fliptri-antialiasing/
float4 FlipTriAntialiasing(in float2 uv)
{
    float4 output;

    bool xOdd = (floor(fmod(uv.x, 2.0)) == 1.0);
    bool yOdd = (floor(fmod(uv.y, 2.0)) == 1.0);
            
    float2 a = float2(xOdd ? -0.5 : 0.5, yOdd ? -0.5 : 0.5);
    float2 b = float2(0.0, yOdd ? 0.5 : -0.5);
    float2 c = float2(xOdd ? 0.5 : -0.5, 0.0);

    a.x /= resolution.x; 
    a.y /= resolution.y;
    b.x /= resolution.x;
    b.y /= resolution.y;
    c.x /= resolution.x;
    c.y /= resolution.y;
        
    output = Texture.Sample(Sampler, uv + a) * 0.2;
    output += Texture.Sample(Sampler, uv + b) * 0.4;
    output += Texture.Sample(Sampler, uv + c) * 0.4;
        
    return output;
}

//4-Rook Antialiasing
//https://blog.demofox.org/2015/04/23/4-rook-antialiasing-rgss/
/*
const float S = 1.0 / 8.0 / resolution.x;
const float L = 3.0 / 8.0 / resolution.y;
output = Texture.Sample(Sampler, In.uv + float2(S, -L)) / 4.0; //GetLensDistortion(In.uv + float2(S, -L)) / 4.0;
output += Texture.Sample(Sampler, In.uv + float2(L, S)) / 4.0; //GetLensDistortion(In.uv + float2(L, S)) / 4.0;
output += Texture.Sample(Sampler, In.uv + float2(-S, -L)) / 4.0; //GetLensDistortion(In.uv + float2(-S, -L)) / 4.0;
output += Texture.Sample(Sampler, In.uv + float2(-L, -S)) / 4.0; //GetLensDistortion(In.uv + float2(-L, -S)) / 4.0;
*/

//LUT
float4 ApplyLUT(float4 color)
{
	//LUTテクスチャサイズの取得
	uint3 LUTSize;
	LutTexture.GetDimensions(0, LUTSize.x, LUTSize.y, LUTSize.z);	
	
	float2 sampleP[2];
	float2 lutScale = 1.0f / LUTSize.xy; //LUT解像度を0~1にするスケール値
	
	color.rgb = saturate(color.rgb) * (LUTSize.y - 1.0f); //LUTの縦解像度に色をスケーリング
	float floorB = floor(color.b);
	
	sampleP[0] = (color.rg + 0.5f) * lutScale; //RG要素をUV値(0~1)に
	sampleP[0].x += floorB * lutScale.y; //Blue要素分xをずらす
	
	sampleP[1] = sampleP[0]; //もう一つのサンプル座標
	sampleP[1].x += lutScale.y; //Blue要素一つ分xをずらす
	
	//サンプリング
	//ブルー要素一つ分ずらしたものと補完	
	return float4(lerp(LutTexture.SampleLevel(Sampler, sampleP[0], 0), LutTexture.SampleLevel(Sampler, sampleP[1], 0), color.b - floorB), color.a);
}

//ピクセルシェーダ
float4 PSMain(PSInput In) : SV_Target0
{	
#if LENS_DISTORTION
	//レンズ歪みを適用した座標を取得
    In.uv = GetLensDistortionUV(In.uv);
#endif
	
#if ANTI_ALIASING
	//FlipTriAntialiasingで色を取得
    float4 output = FlipTriAntialiasing(In.uv);
#else
    float4 output = Texture.Sample(Sampler, In.uv);
#endif
	
#if LUT
	//LUTを適用
	output = ApplyLUT(output);
#endif

#if LENS_DISTORTION
	//周辺減光を適用
    return Vignette(output, In.uv);
#else
    return output;
#endif
}