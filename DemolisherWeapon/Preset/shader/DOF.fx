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

cbuffer PSCb : register(b0) {
	float focus;
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
	float depth = PosMap.Sample(Sampler, In.uv).w;
	
	float fade;
	if (depth > focus) {
		fade = (depth - focus) / (Far - focus);
		//return Normal;
	}
	else {
		fade = (focus - depth) / (focus - Near);
		//return float4(1.0f,0.0f,0.0f,1.0f);
	}
	fade = saturate(fade);

	return lerp(Normal,Resize,fade);
}