Texture2D<float4> Texture : register(t0);

sampler Sampler : register(s0);

struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

//���_�V�F�[�_
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
};

//�r�l�b�g(���ӌ���)���n�܂�ʒu
static const float VIGNETTE_POS_MIN = 0.02f;
static const float VIGNETTE_POS_MAX = 1.0f - VIGNETTE_POS_MIN;
//�I���ʒu
static const float VIGNETTE_END_MIN = -0.006f;

/// @brief �����Y�c�݂�K�p����UV���擾
//http://hikita12312.hatenablog.com/entry/2018/04/07/142219
float2 GetLensDistortionUV(in float2 uv)
{
	// Aspect����l��������ԏ�ł�,�������S����̋����x�N�g��
	float2 rVec = (uv - float2(0.5f, 0.5f));
	rVec.x *= ASPECT_RATIO;
	const float r2 = dot(rVec, rVec);
	const float r4 = r2 * r2;
	const float k1 = 0.0f;
	const float k2 = 0.0f;
	const float k3 = 0.0f;

	// ���a�����c�݂��l�����������x�N�g��
	float2 distortionR = (1.0f + k1 * r2 + k2 * r4) / (1.0f + k3 * r2 + k4 * r4) * rVec;
	
	// �~�������c�ݗʂ̉��Z
	/*const float p1 = 0.0f;
	const float p2 = 0.0f;
	const float C = 2.0f * rVec.x * rVec.y;
	const float2 V = 2.0f * rVec * rVec;
	distortionR += float2(p1*C + p2 * (r2 + V.x), p2*C + p1 * (r2 + V.y));*/

	// �A�X�y�N�g��␳
	distortionR.x *= INV_ASPECT_RATIO;

	// ��ʒ��S��̊g��,UV���W�ւ̕ϊ�
	uv = (distortionR*LENS_DISTORTION_UV_MAGNIFICATION) + float2(0.5f, 0.5f);
	
	return uv;
}

//���ӌ���
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

    a.x /= 1280.0f;//TODO
    a.y /= 720.0f;
    b.x /= 1280.0f;
    b.y /= 720.0f;
    c.x /= 1280.0f;
    c.y /= 720.0f;
        
    output = Texture.Sample(Sampler, uv + a) * 0.2;
    output += Texture.Sample(Sampler, uv + b) * 0.4;
    output += Texture.Sample(Sampler, uv + c) * 0.4;
        
    return output;
}

//4-Rook Antialiasing
//https://blog.demofox.org/2015/04/23/4-rook-antialiasing-rgss/
/*
const float S = 1.0 / 8.0 / 1280.0f;
const float L = 3.0 / 8.0 / 720.0f;
output = Texture.Sample(Sampler, In.uv + float2(S, -L)) / 4.0; //GetLensDistortion(In.uv + float2(S, -L)) / 4.0;
output += Texture.Sample(Sampler, In.uv + float2(L, S)) / 4.0; //GetLensDistortion(In.uv + float2(L, S)) / 4.0;
output += Texture.Sample(Sampler, In.uv + float2(-S, -L)) / 4.0; //GetLensDistortion(In.uv + float2(-S, -L)) / 4.0;
output += Texture.Sample(Sampler, In.uv + float2(-L, -S)) / 4.0; //GetLensDistortion(In.uv + float2(-L, -S)) / 4.0;
*/

float4 PSMain(PSInput In) : SV_Target0
{
	//����̃R�����g�O���Ɩʔ������ƂɂȂ�
	//In.uv = GetLensDistortion(In.uv);

	//��
	//return GetLensDistortion(In.uv);    
	
	//�����Y�c�݂�K�p�������W���擾
    In.uv = GetLensDistortionUV(In.uv);
	//FlipTriAntialiasing�ŐF���擾
    float4 output = FlipTriAntialiasing(In.uv);
	//���ӌ�����K�p
    return Vignette(output, In.uv);
}