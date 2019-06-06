#include"model.fx"

#if defined(INSTANCING)
//�C���X�^���V���O�p�e�N�X�`���C���f�b�N�X
StructuredBuffer<int2> InstancingImposterTextureIndex : register(t7);
#endif

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

	AlbedoRender(In, Out);

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
	finalOut.normal = Out.normal; finalOut.normal.xy *= -1.0f; finalOut.normal *= 0.5f; finalOut.normal += 0.5f;
	finalOut.lightingParam.x = Out.lightingParam.x*0.05f;	//�G�~�b�V�u
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
	
	//�C���f�b�N�X����uv���W���Z�o
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;
#if defined(INSTANCING)
	In.TexCoord.x += (1.0f / imposterPartNum.x) * InstancingImposterTextureIndex[In.instanceID].x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * InstancingImposterTextureIndex[In.instanceID].y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * imposterIndex.y;
#endif

	//�A���x�h
#if ALBEDO_MAP
	//if(Out.albedo.a < 0.1f)
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord);
	Out.albedo *= albedoScale;//�X�P�[����������
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

	NormalRender(In, Out);

	PosRender(In, Out);

	//���C�e�B���O�p�p�����[�^�[
#if LIGHTING_MAP
	float4 lightMap = LightingTexture.Sample(Sampler, In.TexCoord);
	Out.lightingParam.x = lightMap.x / 0.05f * emissive;	//�G�~�b�V�u
	Out.lightingParam.y = lightMap.w * isLighting;			//���C�e�B���O���邩?
	Out.lightingParam.z = lightMap.y * metallic;			//���^���b�N
	Out.lightingParam.w = lightMap.z * shininess;			//�V���C�j�l�X
#endif

	MotionRender(In, Out);

	return Out;
}

#if defined(TEXTURE)
//�C���|�X�^�[��Z�l�o��
float4 PSMain_ImposterRenderZ(ZPSInput In) : SV_Target0
{
	//�C���f�b�N�X����uv���W���Z�o
	In.TexCoord.x /= imposterPartNum.x;
	In.TexCoord.y /= imposterPartNum.y;
#if defined(INSTANCING)
	In.TexCoord.x += (1.0f / imposterPartNum.x) * InstancingImposterTextureIndex[In.instanceID].x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * InstancingImposterTextureIndex[In.instanceID].y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * imposterIndex.y;
#endif

	//�A���x�h
	float alpha = albedoTexture.Sample(Sampler, In.TexCoord).a * albedoScale.a;

	//���e�X�g
	if (alpha > 0.5f) {
	}
	else {
		discard;
	}

	return In.posInProj.z / In.posInProj.w + depthBias.x;
}
#endif