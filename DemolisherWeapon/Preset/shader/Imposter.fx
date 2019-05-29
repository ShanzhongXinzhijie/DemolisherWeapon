#include"model.fx"

//�C���|�X�^�[�̏o��
struct PSOutput_RenderImposter {
	float4 albedo		: SV_Target0;		//�A���x�h
	float3 normal		: SV_Target1;		//�@��
	float4 lightingParam: SV_Target2;		//���C�e�B���O�p�p�����[�^�[
};
PSOutput_RenderImposter PSMain_RenderImposter(PSInput In)
{
	PSOutput_RenderImposter finalOut = (PSOutput_RenderImposter)0;
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	//�A���x�h
	//���j�A��Ԃւ̕ϊ��͂��Ȃ�
#if ALBEDO_MAP
	//�ʏ�
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);
	Out.albedo *= albedoScale;//�X�P�[����������
#else
	//�A���x�h�e�N�X�`�����Ȃ��ꍇ�̓X�P�[�������̂܂܎g��
	Out.albedo = albedoScale;
#endif

	//���e�X�g
	if (Out.albedo.a > 0.5f) {
		Out.albedo.a = 1.0f;//����������
	}
	else {
		discard;//�`�悵�Ȃ�
	}

	NormalRender(In, Out);

	ParamRender(In, Out);

	finalOut.albedo = Out.albedo;
	finalOut.normal = Out.normal; finalOut.normal.xy *= -1.0f;
	finalOut.lightingParam.x = Out.lightingParam.x;	//�G�~�b�V�u
	finalOut.lightingParam.w = Out.lightingParam.y;	//���C�e�B���O���邩?
	finalOut.lightingParam.y = Out.lightingParam.z;	//���^���b�N
	finalOut.lightingParam.z = Out.lightingParam.w;	//�V���C�j�l�X
	return finalOut;
}

//�C���|�X�^�[��GBuffer�ւ̏�������
PSOutput_RenderGBuffer PSMain_ImposterRenderGBuffer(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	/*
#if ALBEDO_MAP && NORMAL_MAP
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);
#endif
	if (In.TexCoord.x < 0.01f || In.TexCoord.x > 1.0f - 0.01f || -In.TexCoord.y < 0.01f || -In.TexCoord.y > 1.0f - 0.01f) {
		Out.albedo = float4(-In.TexCoord.y, 0, 0, 1);
		return Out;
	}
	*/
	
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;

	In.TexCoord.x += (1.0f / imposterPartNum.x) * imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * imposterIndex.y;

#if ALBEDO_MAP && NORMAL_MAP
	//if(Out.albedo.a < 0.1f)
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord);
	Out.albedo.xyz = pow(Out.albedo.xyz, 2.2f);//���j�A��Ԃɕϊ�
#else
	discard;
#endif

	//���e�X�g
	if (Out.albedo.a > 0.5f) {
		Out.albedo.a = 1.0f;//����������
	}
	else {
		discard;//�`�悵�Ȃ�
	}

#if ALBEDO_MAP && NORMAL_MAP
	Out.normal = NormalTexture.Sample(Sampler, In.TexCoord);
	//Out.normal = Out.normal * 2.0f - 1.0f;
	Out.normal = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal = normalize(Out.normal);
#endif

	PosRender(In, Out);//����ł����̂�

	ParamRender(In, Out);

	MotionRender(In, Out);

	return Out;
}