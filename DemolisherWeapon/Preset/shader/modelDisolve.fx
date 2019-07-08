#ifndef DW_MODEL_DISOLVE_FX
#define DW_MODEL_DISOLVE_FX

#include"model.fx"

//�������l�e�N�X�`��
Texture2D<float> DisolveTexture : register(t9);
//�f�B�]���u�V�F�[�_�p�������l
StructuredBuffer<float> Disolve_t : register(t10);

//�f�B�]���u�V�F�[�_(GBuffer)
PSOutput_RenderGBuffer PSMain_DisolveGBuffer(PSInput In)
{
	if (DisolveTexture.Sample(Sampler, In.TexCoord) > Disolve_t[In.instanceID]) {
		return PSMain_RenderGBuffer(In);//�ʏ�V�F�[�f�B���O
	}
	else {
		discard;//��
	}
}
//�f�B�]���u�V�F�[�_(Z�l�o��)
float4 PSMain_DisolveZ(ZPSInput In) : SV_Target0
{
	if (DisolveTexture.Sample(Sampler, In.TexCoord) > Disolve_t[In.instanceID]) {
		return PSMain_RenderZ(In);//�ʏ�V�F�[�f�B���O
	}
	else {
		discard;//��
	}
}

#endif //DW_MODEL_DISOLVE_FX