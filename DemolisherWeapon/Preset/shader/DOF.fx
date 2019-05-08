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

	//�J�����ݒ�
	float camNear;
	float camFar;
};

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = In.pos;
	psIn.uv = In.uv;
	return psIn;
}

//�f�v�X�l����`�ɕϊ�
float LinearizeDepth(float depth)
{
	return (2.0 * camNear) / (camFar + camNear - depth * (camFar - camNear));
}

float4 PSMain(PSInput In) : SV_Target0
{
	float4 Resize = ResizeBuffer.Sample(Sampler, In.uv);
	float4 Normal = FRT.Sample(Sampler, In.uv);
	float depth = LinearizeDepth(PosMap.Sample(Sampler, In.uv).w);
	
	float fade = 0.0f;
	if (depth > focus + focusArea) {
		fade = abs(depth - (focus + focusArea)) / abs(Far - (focus + focusArea));
	}
	else if (depth < focus - focusArea) {
		fade = abs((focus - focusArea) - depth) / abs((focus - focusArea) - Near);
	}
	fade = saturate(fade);

	//return float4(fade, 0.0f, 1.0f, 1.0f);
	return lerp(Normal, Resize, fade);
}