#include"model.fx"

//���f���T�C�Y(�J���������ւ�)
StructuredBuffer<float> ImposterSizeToCamera : register(t7);
#if defined(INSTANCING)
//�C���X�^���V���O�p�C���|�X�^�[�g�嗦
StructuredBuffer<float> InstancingImposterScale : register(t8);
#endif

static const float PI = 3.14159265359f;
static const float PI2 = PI * 2.0f;

//�C���|�X�^�[�e�N�X�`���̏o��
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

//�C���|�X�^�[�̌v�Z
void CalcImposter(out int2 out_index, out float4x4 out_rotMat, out float3 out_offsetPos
#if defined(INSTANCING)
	, in uint instanceID
#endif
) {
	//�C���|�X�^�[�̍��W
#if defined(INSTANCING)
	float3 pos = float3(InstancingWorldMatrix[instanceID]._m03, InstancingWorldMatrix[instanceID]._m13, InstancingWorldMatrix[instanceID]._m23);
#else
	float3 pos = float3(mWorld._m03, mWorld._m13, mWorld._m23);
#endif

	//�C���|�X�^�[�p�C���f�b�N�X�v�Z
	float3 polyDir = normalize(camWorldPos - pos);

	//X����]
	float3 axisDir = polyDir; axisDir.x = length(float2(polyDir.x, polyDir.z));
	float XRot = atan2(axisDir.y, axisDir.x);
	out_index.y = (int)round(XRot / PI * imposterPartNum.y) - (int)(imposterPartNum.y / 2.0f - 0.5f);

	//Y����]		
	float YRot = atan2(polyDir.x, polyDir.z);
	out_index.x = (int)round(-YRot / PI2 * imposterPartNum.x) + (int)(imposterPartNum.x / 2.0f - 0.5f);

	//��]		
	float r = -out_index.y * (PI / (imposterPartNum.y - 1)) - PI * 0.5f;
	float sinr = sin(r), cosr = cos(r);
	float4x4 rotX = {1.0f,0.0f,0.0f,0.0f, //��X����]
					0.0f,cosr,sinr,0.0f,
					0.0f,-sinr,cosr,0.0f,
					0.0f,0.0f,0.0f,1.0f,
					};
	r = out_index.x * (PI2 / (imposterPartNum.x - 1));
	sinr = sin(r), cosr = cos(r);
	float4x4 rotY = /*{cosr,sinr,0.0f,0.0f, //��Z����]
					-sinr,cosr,0.0f,0.0f,
					0.0f,0.0f,1.0f,0.0f,
					0.0f,0.0f,0.0f,1.0f,
					};*/
					{cosr, 0.0f, -sinr, 0.0f, //��Y����]
					0.0f, 1.0f, 0.0f, 0.0f,
					sinr, 0.0f, cosr, 0.0f,
					0.0f,0.0f,0.0f,1.0f,
					};

	//��]�s��̍쐬
	out_rotMat = mul(rotY, rotX);
	//�J���������Ƀ��f���T�C�Y�����W���炷
	//�����܂�h�~
#if defined(INSTANCING)	
	out_offsetPos = polyDir * (InstancingImposterScale[instanceID] * ImposterSizeToCamera[(imposterPartNum.y - 1 + out_index.y)*imposterPartNum.x + out_index.x]);
#else
	out_offsetPos = polyDir * (imposterScale * ImposterSizeToCamera[(imposterPartNum.y - 1 + out_index.y)*imposterPartNum.x + out_index.x]);
#endif
}
//TODO �r���{�[�h��

//���_�V�F�[�_(�ʏ�)
PSInput VSMain_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	//�C���|�X�^�[���̌v�Z
	int2 index;
	float4x4 rotMat;
	float3 offsetPos, offsetPosOld;
	CalcImposter(index, rotMat, offsetPos
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//���W����
	offsetPosOld = offsetPos;
#if defined(INSTANCING)
	offsetPos += float3(InstancingWorldMatrix[instanceID]._m03, InstancingWorldMatrix[instanceID]._m13, InstancingWorldMatrix[instanceID]._m23);
	offsetPosOld += float3(InstancingWorldMatrixOld[instanceID]._m03, InstancingWorldMatrixOld[instanceID]._m13, InstancingWorldMatrixOld[instanceID]._m23);
#else
	offsetPos += float3(mWorld._m03, mWorld._m13, mWorld._m23);
	offsetPosOld += float3(mWorld_old._m03, mWorld_old._m13, mWorld_old._m23);
#endif

	//�s�񍇐�
	float4x4 worldMat, worldMatOld;
#if defined(INSTANCING)
	worldMat = mul(rotMat, InstancingWorldMatrix[instanceID]);
	worldMatOld = mul(rotMat, InstancingWorldMatrixOld[instanceID]);
#else
	worldMat = mul(rotMat, mWorld);
	worldMatOld = mul(rotMat, mWorld_old);	
#endif
	worldMat._m03 = offsetPos.x; worldMat._m13 = offsetPos.y; worldMat._m23 = offsetPos.z;
	worldMatOld._m03 = offsetPosOld.x; worldMatOld._m13 = offsetPosOld.y; worldMatOld._m23 = offsetPosOld.z;
	
	//�ʏ폈��
	PSInput psInput = VSModel(In, worldMat, worldMatOld
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//�C���f�b�N�X�ݒ�
	psInput.imposterIndex = index;
	
	return psInput;
}
//���_�V�F�[�_(�[�x�l)
ZPSInput VSMain_RenderZ_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {	
	//�C���|�X�^�[���̌v�Z
	int2 index;
	float4x4 worldMat;
	float3 pos;
	CalcImposter(index, worldMat, pos
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//���W����
#if defined(INSTANCING)
	pos = float3(InstancingWorldMatrix[instanceID]._m03, InstancingWorldMatrix[instanceID]._m13, InstancingWorldMatrix[instanceID]._m23);
#else
	pos = float3(mWorld._m03, mWorld._m13, mWorld._m23);
#endif

	//�s�񍇐�
#if defined(INSTANCING)
	worldMat = mul(worldMat, InstancingWorldMatrix[instanceID]);
#else
	worldMat = mul(worldMat, mWorld);
#endif
	worldMat._m03 = pos.x; worldMat._m13 = pos.y; worldMat._m23 = pos.z;

	//�ʏ폈��
	ZPSInput psInput = VSModel_RenderZ(In, worldMat
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	//�C���f�b�N�X�ݒ�
	psInput.imposterIndex = index;

	return psInput;
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
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;

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
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;

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