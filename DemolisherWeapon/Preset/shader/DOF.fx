struct VSInput {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

Texture2D<float4> FRT : register(t0);			//�ŏI�����_�[�^�[�Q�b�g
Texture2D<float4> ResizeBuffer : register(t1);	//�k���o�b�t�@
Texture2D<float4> PosMap : register(t2);		//���W�}�b�v(w�̐[�x�l���g�p)
sampler Sampler : register(s0);

//�萔�o�b�t�@
//[DepthOfFieldRender.h : SPSConstantBuffer]
cbuffer PSCb : register(b0) {
	float focus;
	float focusArea;
	float Near;
	float Far;
};

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = In.pos;
	psIn.uv = In.uv;
	return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
	float4 Resize = ResizeBuffer.Sample(Sampler, In.uv);
	float4 Normal = FRT.Sample(Sampler, In.uv);
    float depth = length(PosMap.Sample(Sampler, In.uv).xyz);
	
	float fade = 0.0f;
	if (depth > focus + focusArea) {
		fade = abs(depth - (focus + focusArea)) / abs(Far - (focus + focusArea));
	}
	else if (depth < focus - focusArea) {
		fade = abs((focus - focusArea) - depth) / abs((focus - focusArea) - Near);
	}
	fade = saturate(fade);

	//�m�C�Y�e�X�g
	//Interleaved gradient noise
	/*float3 magic = float3(0.06711056, 0.00583715, 52.9829189);
	float noise = frac(magic.z * frac(dot(In.pos, magic.xy)));
	return float4(noise, noise, noise, 1.0f);*/

	//return float4(fade, 0.0f, 1.0f, 1.0f);
	return lerp(Normal, Resize, fade);
}