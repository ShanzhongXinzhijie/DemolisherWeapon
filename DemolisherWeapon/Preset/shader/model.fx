/*!
 * @brief	���f���V�F�[�_�[�B
 */
#ifndef DW_MODEL_FX
#define DW_MODEL_FX

#include"MotionBlurHeader.h"

#if defined(ALL_VS)
#define MOTIONBLUR 1
#define NORMAL_MAP 1
#endif

/////////////////////////////////////////////////////////////
// Shader Resource View
// [SkinModelShaderConst.h:EnSkinModelSRVReg]
/////////////////////////////////////////////////////////////

//�e�N�X�`��
#if ALBEDO_MAP || defined(TEXTURE) || defined(SKY_CUBE)
#if !defined(SKY_CUBE)
//�A���x�h�e�N�X�`��
Texture2D<float4> albedoTexture : register(t0);	
#else
//�X�J�C�{�b�N�X�p�L���[�u�}�b�v
TextureCube<float4> skyCubeMap : register(t0);
#endif
#endif

#if NORMAL_MAP
//�m�[�}���}�b�v
Texture2D<float3> NormalTexture : register(t1);
#endif

#if LIGHTING_MAP
//���C�e�B���O�p�����[�^�}�b�v
Texture2D<float4> LightingTexture : register(t2);
#endif

#if TRANSLUCENT_MAP
//�g�����X���[�Z���g�}�b�v
Texture2D<float> TranslucentTexture : register(t11);
#endif

#if defined(SOFT_PARTICLE)
//�r���[���W�e�N�X�`��(w���[�x�l)
Texture2D<float4> ViewPosTexture : register(t12);
#endif

//�X�g���N�`���[�o�b�t�@

//�{�[���s��
StructuredBuffer<float4x4> boneMatrix : register(t3);
StructuredBuffer<float4x4> boneMatrixOld : register(t4);
#if defined(INSTANCING)
//�C���X�^���V���O�p���[���h�s��
StructuredBuffer<float4x4> InstancingWorldMatrix : register(t5);
StructuredBuffer<float4x4> InstancingWorldMatrixOld : register(t6);
#endif

/////////////////////////////////////////////////////////////
// SamplerState
/////////////////////////////////////////////////////////////
sampler Sampler : register(s0);

/////////////////////////////////////////////////////////////
// �萔�o�b�t�@�B
// [SkinModelShaderConst.h:EnSkinModelCBReg]
/////////////////////////////////////////////////////////////
/*!
 * @brief	���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[�p�̒萔�o�b�t�@�B
 * [SkinModel.h:SVSConstantBuffer]
 */
cbuffer VSPSCb : register(b0){
	//�s��
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;

	//�O��̍s��
	float4x4 mWorld_old;
	float4x4 mView_old;
	float4x4 mProj_old;

	//�J�����̈ړ���
	float4 camMoveVec;//w:�������l�������X�P�[��

	//�[�x�l�̃o�C�A�X
	float4 depthBias;//x:max=(1.0f) y:max=(far-near) z:�u���[�̋ߋ����������l

	//�J�����̃��[���h���W
	float3 camWorldPos;

	//�Ȃ�
    float cb_t;

	//�C���|�X�^�[�p
	int2 imposterPartNum;//������
	float2 imposterParameter;//x:�X�P�[��,y:Y����]

    //�J������Near(x)�EFar(y)
	float2 nearFar;

	//�\�t�g�p�[�e�B�N�����L���ɂȂ�͈�
    float softParticleArea;
	
	//���[�V�����u���[�X�P�[��
	float MotionBlurScale;
};

//�萔�o�b�t�@�@[MaterialSetting.h:MaterialParam]
//�}�e���A���p�����[�^�[
cbuffer MaterialCb : register(b1) {
	float4 albedoScale;	//�A���x�h�ɂ�����X�P�[��
	float  emissive;	//���Ȕ���
	float  isLighting;	//���C�e�B���O���邩
	float  metallic;	//���^���b�N
	float  shininess;	//�V���C�l�X(���t�l�X�̋t)
	float2 uvOffset;	//UV���W�I�t�Z�b�g
	float  triPlanarMapUVScale;//TriPlanarMapping����UV���W�ւ̃X�P�[��
    float  translucent; //�g�����X���[�Z���g(���̓��ߋ)
}

/////////////////////////////////////////////////////////////
//�e��\����
/////////////////////////////////////////////////////////////
/*!
 * @brief	�X�L���Ȃ����f���̒��_�\���́B
 */
struct VSInputNmTxVcTangent
{
    float4 Position : SV_Position;			//���_���W�B
    float3 Normal   : NORMAL;				//�@���B
    float3 Tangent  : TANGENT;				//�ڃx�N�g���B
	//float3 Binormal : BINORMAL;			//�]�@���B
    float2 TexCoord : TEXCOORD0;			//UV���W�B
};
/*!
 * @brief	�X�L�����胂�f���̒��_�\���́B
 */
struct VSInputNmTxWeights
{
    float4 Position : SV_Position;			//���_���W�B
    float3 Normal   : NORMAL;				//�@���B
    float2 TexCoord	: TEXCOORD0;			//UV���W�B
    float3 Tangent	: TANGENT;				//�ڃx�N�g���B
	//float3 Binormal : BINORMAL;			//�]�@���B
	uint4  Indices  : BLENDINDICES0;		//���̒��_�Ɋ֘A�t������Ă���{�[���ԍ��Bx,y,z,w�̗v�f�ɓ����Ă���B4�{�[���X�L�j���O�B
    float4 Weights  : BLENDWEIGHT0;			//���̒��_�Ɋ֘A�t������Ă���{�[���ւ̃X�L���E�F�C�g�Bx,y,z,w�̗v�f�ɓ����Ă���B4�{�[���X�L�j���O�B
};

/*!
 * @brief	�s�N�Z���V�F�[�_�[�̓��́B
 */
struct PSInput{
	//��{���
	float4 Position 	: SV_POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float2 TexCoord 	: TEXCOORD0;

	//���W
	float3 Viewpos		: TEXCOORD1;
	float3 Localpos		: TEXCOORD2;
    float3 Worldpos		: TEXCOORD3;

	float4 curPos		: CUR_POSITION;//���ݍ��W
	float4 lastPos		: LAST_POSITION;//�ߋ����W
	bool isWorldMove	: IS_WORLD_BLUR;//���[���h��Ԃňړ����Ă��邩?

	uint instanceID		: InstanceID;//�C���X�^���XID

	float3 cubemapPos	: CUBE_POS;//�X�J�C�{�b�N�X�̃e�N�X�`���擾�p

	int2 imposterIndex	: IMPOSTER_INDEX;//�C���|�X�^�[�p�C���f�b�N�X
};

//Z�l�������݃s�N�Z���V�F�[�_�[�̓���
struct ZPSInput {
	//��{���
	float4 Position 	: SV_POSITION;
	float2 TexCoord 	: TEXCOORD0;
	float4 posInProj	: TEXCOORD1;

	uint instanceID		: InstanceID;//�C���X�^���XID

	int2 imposterIndex	: IMPOSTER_INDEX;//�C���|�X�^�[�p�C���f�b�N�X
};

//G-Buffer�o��
struct PSOutput_RenderGBuffer {
	float4 albedo		: SV_Target0;		//�A���x�h
	float4 normal		: SV_Target1;		//�@��
	float4 viewpos		: SV_Target2;		//�r���[���W
	float4 velocity		: SV_Target3;		//���x
	float4 velocityPS	: SV_Target4;		//���x(�s�N�Z���V�F�[�_)
	float4 lightingParam: SV_Target5;		//���C�e�B���O�p�p�����[�^�[
};

/////////////////////////////////////////////////////////////
// �֐�
/////////////////////////////////////////////////////////////
//�X�L���Ȃ����f���̒��_�V�F�[�_�p�֐�
PSInput VSModel( VSInputNmTxVcTangent In, float4x4 worldMat, float4x4 worldMatOld
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	PSInput psInput = (PSInput)0;

#if defined(INSTANCING)
	//�C���X�^���XID�L�^
	psInput.instanceID = instanceID;
#endif

	//���[���h�s��K��
	float4 pos = mul(worldMat, In.Position);
	float3 posW = pos.xyz;
    psInput.Localpos = In.Position;
    psInput.Worldpos = posW;

	//�X�J�C�{�b�N�X�p���
#if defined(SKY_CUBE)
	psInput.cubemapPos = normalize(posW - camWorldPos);
#endif

	//�r���[�v���W�F�N�V�����s��K��
	pos = mul(mView, pos); psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);

	//�ݒ�
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;

	//�@�����
	psInput.Normal = normalize(mul(worldMat, In.Normal));
#if NORMAL_MAP
	psInput.Tangent = normalize(mul(worldMat, In.Tangent));
	psInput.Binormal = normalize(cross(psInput.Normal, psInput.Tangent));// normalize(mul(worldMat, In.Binormal));
#endif

	//�ϊ�����W
	psInput.curPos = pos;

	//�x���V�e�B�}�b�v�p���
#if MOTIONBLUR
	float4 oldpos = mul(worldMatOld, In.Position);
	oldpos.xyz = lerp(posW, oldpos.xyz, MotionBlurScale);

	float3 trans = float3(worldMat._m03, worldMat._m13, worldMat._m23);
	float3 transOld = float3(worldMatOld._m03, worldMatOld._m13, worldMatOld._m23);
	transOld = lerp(trans, transOld, MotionBlurScale);
	trans -= transOld;

	if (length(trans) > camMoveVec.w
		&& distance(camMoveVec.xyz, trans) > camMoveVec.w
		|| distance(float3(worldMat._m00, worldMat._m10, worldMat._m20), float3(worldMatOld._m00, worldMatOld._m10, worldMatOld._m20)) > 0.0f
		|| distance(float3(worldMat._m01, worldMat._m11, worldMat._m21), float3(worldMatOld._m01, worldMatOld._m11, worldMatOld._m21)) > 0.0f
		|| distance(float3(worldMat._m02, worldMat._m12, worldMat._m22), float3(worldMatOld._m02, worldMatOld._m12, worldMatOld._m22)) > 0.0f
	){
		psInput.isWorldMove = true;
	}

	float4 oldpos2 = mul(mView_old, oldpos);
	oldpos = mul(mView, oldpos);
	oldpos.xyz = lerp(oldpos.xyz, oldpos2.xyz, MotionBlurScale);

	oldpos = mul(mProj_old, oldpos);
		
	psInput.lastPos = oldpos;
#endif

	return psInput;
}

/*!--------------------------------------------------------------------------------------
 * @brief	�X�L���Ȃ����f���p�̒��_�V�F�[�_�[�B
-------------------------------------------------------------------------------------- */
PSInput VSMain(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
#if defined(INSTANCING)
	return VSModel(In, InstancingWorldMatrix[instanceID], InstancingWorldMatrixOld[instanceID], instanceID);
#else
	return VSModel(In, mWorld, mWorld_old);
#endif 
}

//Z�l�������ݗp�X�L���Ȃ����f�����_�V�F�[�_�p�֐�
ZPSInput VSModel_RenderZ(VSInputNmTxVcTangent In, float4x4 worldMat
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	ZPSInput psInput = (ZPSInput)0;

#if defined(INSTANCING)
	//�C���X�^���XID�L�^
	psInput.instanceID = instanceID;
#endif

	float4 pos = mul(worldMat, In.Position);
	pos = mul(mView, pos);
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;

	return psInput;
}

//Z�l�������ݗp�X�L���Ȃ����f�����_�V�F�[�_�[
ZPSInput VSMain_RenderZ(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
#if defined(INSTANCING)
	return VSModel_RenderZ(In, InstancingWorldMatrix[instanceID], instanceID);
#else
	return VSModel_RenderZ(In, mWorld);
#endif 
}

/*!--------------------------------------------------------------------------------------
 * @brief	�X�L�����胂�f���p�̒��_�V�F�[�_�[�B
 * �S�Ă̒��_�ɑ΂��Ă��̃V�F�[�_�[���Ă΂��B
 * In��1�̒��_�f�[�^�BVSInputNmTxWeights�����Ă݂悤�B
-------------------------------------------------------------------------------------- */
PSInput VSMainSkin( VSInputNmTxWeights In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	PSInput psInput = (PSInput)0;
	///////////////////////////////////////////////////
	//��������X�L�j���O���s���Ă���ӏ��B
	//�X�L���s����v�Z�B
	///////////////////////////////////////////////////
	float4x4 skinning = 0;	
	float4 pos = 0;
	{	
		float w = 0.0f;
		[unroll]
	    for (int i = 0; i < 3; i++)
	    {
			//boneMatrix�Ƀ{�[���s�񂪐ݒ肳��Ă��āA
			//In.indices�͒��_�ɖ��ߍ��܂ꂽ�A�֘A���Ă���{�[���̔ԍ��B
			//In.weights�͒��_�ɖ��ߍ��܂ꂽ�A�֘A���Ă���{�[���̃E�F�C�g�B
	        skinning += boneMatrix[In.Indices[i]] * In.Weights[i];
	        w += In.Weights[i];
	    }
	    //�Ō�̃{�[�����v�Z����B
	    skinning += boneMatrix[In.Indices[3]] * (1.0f - w);	  	
	}

#if defined(INSTANCING)
	//�C���X�^���XID�L�^
	psInput.instanceID = instanceID;
	//�X�L���s��ƃ��[���h�s�����Z
	skinning = mul(InstancingWorldMatrix[instanceID], skinning);
#endif
	
	//���_���W�ɃX�L���s�����Z���āA���_�����[���h��Ԃɕϊ��B
	//mul�͏�Z���߁B
	pos = mul(skinning, In.Position);
	psInput.Normal = normalize( mul(skinning, In.Normal) );
#if NORMAL_MAP
	psInput.Tangent = normalize( mul(skinning, In.Tangent) );
	psInput.Binormal = normalize(cross(psInput.Normal, psInput.Tangent));//normalize( mul(skinning, In.Binormal) );
#endif

	float3 posW = pos.xyz; 
#if defined(SKY_CUBE)
	psInput.cubemapPos = normalize(posW - camWorldPos);
#endif
    psInput.Localpos = In.Position;
    psInput.Worldpos = posW;

	pos = mul(mView, pos);  psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;

	psInput.curPos = pos;

	//�x���V�e�B�}�b�v�p���
#if MOTIONBLUR
		float4x4 oldskinning = 0;
		float4 oldpos = 0;
		{
			float w = 0.0f;
			[unroll]
			for (int i = 0; i < 3; i++)
			{
				oldskinning += boneMatrixOld[In.Indices[i]] * In.Weights[i];
				w += In.Weights[i];
			}
			oldskinning += boneMatrixOld[In.Indices[3]] * (1.0f - w);
		}
#if defined(INSTANCING)
		//�C���X�^���V���O
		oldskinning = mul(InstancingWorldMatrixOld[instanceID], oldskinning);
#endif
		oldpos = mul(oldskinning, In.Position);
		oldpos.xyz = lerp(posW, oldpos.xyz, MotionBlurScale);

		float3 trans = float3(skinning._m03, skinning._m13, skinning._m23);
		float3 transOld = float3(oldskinning._m03, oldskinning._m13, oldskinning._m23);
		transOld = lerp(trans, transOld, MotionBlurScale);
		trans -= transOld;

		if (length(trans) > camMoveVec.w
			&& distance(camMoveVec.xyz, trans) > camMoveVec.w
			|| distance(float3(skinning._m00, skinning._m10, skinning._m20), float3(oldskinning._m00, oldskinning._m10, oldskinning._m20)) > 0.0f
			|| distance(float3(skinning._m01, skinning._m11, skinning._m21), float3(oldskinning._m01, oldskinning._m11, oldskinning._m21)) > 0.0f
			|| distance(float3(skinning._m02, skinning._m12, skinning._m22), float3(oldskinning._m02, oldskinning._m12, oldskinning._m22)) > 0.0f
		) {
			psInput.isWorldMove = true;
		}

		float4 oldpos2 = mul(mView_old, oldpos);
		oldpos = mul(mView, oldpos);
		oldpos.xyz = lerp(oldpos.xyz, oldpos2.xyz, MotionBlurScale);

		oldpos = mul(mProj_old, oldpos);
			
		psInput.lastPos = oldpos;
#endif

    return psInput;
}
//Z�l�������ݗp
ZPSInput VSMainSkin_RenderZ(VSInputNmTxWeights In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	ZPSInput psInput = (ZPSInput)0;
	///////////////////////////////////////////////////
	//��������X�L�j���O���s���Ă���ӏ��B
	//�X�L���s����v�Z�B
	///////////////////////////////////////////////////
	float4x4 skinning = 0;
	float4 pos = 0;
	{
		float w = 0.0f;
		[unroll]
		for (int i = 0; i < 3; i++)
		{
			//boneMatrix�Ƀ{�[���s�񂪐ݒ肳��Ă��āA
			//In.indices�͒��_�ɖ��ߍ��܂ꂽ�A�֘A���Ă���{�[���̔ԍ��B
			//In.weights�͒��_�ɖ��ߍ��܂ꂽ�A�֘A���Ă���{�[���̃E�F�C�g�B
			skinning += boneMatrix[In.Indices[i]] * In.Weights[i];
			w += In.Weights[i];
		}
		//�Ō�̃{�[�����v�Z����B
		skinning += boneMatrix[In.Indices[3]] * (1.0f - w);		
	}

#if defined(INSTANCING)
	//�C���X�^���XID�L�^
	psInput.instanceID = instanceID;
	//�X�L���s��ƃ��[���h�s�����Z
	skinning = mul(InstancingWorldMatrix[instanceID], skinning);
#endif

	//���_���W�ɃX�L���s�����Z���āA���_�����[���h��Ԃɕϊ��B
	//mul�͏�Z���߁B
	pos = mul(skinning, In.Position);	
	pos = mul(mView, pos);
	pos = mul(mProj, pos);

	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;

	return psInput;
}

void AlbedoRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//�A���x�h
#if ALBEDO_MAP || defined(SKY_CUBE)
#if !defined(SKY_CUBE)
	//�ʏ�
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);
#else
	//�X�J�C�{�b�N�X
	Out.albedo = skyCubeMap.SampleLevel(Sampler, In.cubemapPos, 0);
#endif
	Out.albedo.xyz = pow(Out.albedo.xyz, 2.2f);//���j�A��Ԃɕϊ�
	Out.albedo *= albedoScale;//�X�P�[����������
#else
	//�A���x�h�e�N�X�`�����Ȃ��ꍇ�̓X�P�[�������̂܂܎g��
	Out.albedo = albedoScale;
#endif
}
void NormalRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//�@��
#if NORMAL_MAP
	Out.normal.xyz = NormalTexture.Sample(Sampler, In.TexCoord + uvOffset);
	Out.normal.xyz = Out.normal.xyz * 2.0f - 1.0f;
	Out.normal.xyz = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal.xyz = normalize(Out.normal.xyz);
#else
	Out.normal.xyz = In.Normal;
#endif
}
void PosRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//�r���[���W
	Out.viewpos = float4(In.Viewpos.x, In.Viewpos.y, In.Viewpos.z + depthBias.y, In.curPos.z / In.curPos.w + depthBias.x);
}
void ParamRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//���C�e�B���O�p�p�����[�^�[
#if LIGHTING_MAP
	//x:�G�~�b�V�u y:���^���b�N z:�V���C�j�l�X w:���C�e�B���O���邩?
	float4 lightMap = LightingTexture.Sample(Sampler, In.TexCoord + uvOffset);
	Out.lightingParam.x = lightMap.x * emissive;	//�G�~�b�V�u
	Out.lightingParam.y = lightMap.w * isLighting;	//���C�e�B���O���邩?
	Out.lightingParam.z = lightMap.y * metallic;	//���^���b�N
	Out.lightingParam.w = lightMap.z * shininess;	//�V���C�j�l�X
#else
	Out.lightingParam.x = emissive;		//�G�~�b�V�u
	Out.lightingParam.y = isLighting;	//���C�e�B���O���邩?
	Out.lightingParam.z = metallic;		//���^���b�N
	Out.lightingParam.w = shininess;	//�V���C�j�l�X
#endif
}
void MotionRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//���x
#if MOTIONBLUR
	float2	current = In.curPos.xy / In.curPos.w;
	float2	last = In.lastPos.xy / In.lastPos.w;

	//����
	if (In.lastPos.z < 0.0f) {
		//if (last.z < 0.0f || last.z > 1.0f) {
		Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocityPS.z = -1.0f;
		Out.velocityPS.w = -1.0f;
		return;//return Out;
		//discard; 
	}

	current.xy *= float2(0.5f, -0.5f); current.xy += 0.5f;
	last.xy *= float2(0.5f, -0.5f); last.xy += 0.5f;

	Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
	Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

	//�I�u�W�F�N�g�������Ă��� or �J�����ɋ߂�
	if (In.isWorldMove || In.curPos.z + depthBias.y < depthBias.z) {
		Out.velocity.xy = current.xy - last.xy;

		Out.velocityPS.z = max(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocityPS.w = -1.0f;//PS�u���[���Ȃ�

		//Out.albedo.r = 1.0f; Out.albedo.b = 0.0f; Out.albedo.g = 0.0f;
	}
	else {
		Out.velocityPS.xy = current.xy - last.xy;

		Out.velocityPS.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocityPS.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

		//������������
		if (abs(Out.velocityPS.x) < BUNBO*0.5f && abs(Out.velocityPS.y) < BUNBO*0.5f) {
			Out.velocityPS.z = Out.velocityPS.w;
			Out.velocityPS.w = -1.0f;//PS�u���[���Ȃ�
		}

		//Out.albedo.r *= 0.1f; Out.albedo.b = 1.0f; Out.albedo.g *= 0.1f;
	}
#else
	Out.velocity.z = In.curPos.z + depthBias.y;
	Out.velocity.w = In.curPos.z + depthBias.y;
	Out.velocityPS.z = -1.0f;// In.curPos.z + depthBias.y;
	Out.velocityPS.w = -1.0f;// In.curPos.z + depthBias.y;
#endif
}
void TranslucentRender(in PSInput In, inout PSOutput_RenderGBuffer Out)
{
#if TRANSLUCENT_MAP
    Out.normal.a = TranslucentTexture.Sample(Sampler, In.TexCoord + uvOffset) * translucent;
#else
    Out.normal.a = translucent;
#endif
}

//GBuffer�o��
PSOutput_RenderGBuffer PSMain_RenderGBuffer(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	AlbedoRender(In,Out);

	//���e�X�g
	if (Out.albedo.a > 0.5f) { 
		Out.albedo.a = 1.0f;//����������
	}
	else {
		discard;//�`�悵�Ȃ�
	}

	NormalRender(In, Out);

	PosRender(In, Out);

	ParamRender(In, Out);

	MotionRender(In, Out);

    TranslucentRender(In, Out);

//#if defined(SKY_CUBE)
//	if(In.cubemapPos.y<0.0f){Out.lightingParam.y=1.0f;}
//#endif

	return Out;
}

#if !defined(SKY_CUBE)
//Z�l�o��
float4 PSMain_RenderZ(ZPSInput In) : SV_Target0
{
#if defined(TEXTURE)
	//�A���x�h
	float alpha = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset).a * albedoScale.a;
#else
	float alpha = albedoScale.a;
#endif
	//���e�X�g
	if (alpha > 0.5f) {
	}
	else {
		discard;
	}
	return In.posInProj.z / In.posInProj.w + depthBias.x ;// +1.0f*max(abs(ddx(In.posInProj.z / In.posInProj.w)), abs(ddy(In.posInProj.z / In.posInProj.w)));
}
#endif

//�f�v�X�l����`�ɕϊ�
float LinearizeDepth(float depth, float near, float far)
{
    return (2.0 * near) / (far + near - depth * (far - near));
}

//�f�ނ̖��V�F�[�_
float4 SozaiNoAziInner(PSInput In)
{
    float4 Out;

#if defined(TEXTURE)
	Out = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset) * albedoScale;
#else
    Out = albedoScale;
#endif
	
//�\�t�g�p�[�e�B�N��
#if defined(SOFT_PARTICLE)
	//���f���[�x�l�Z�o
    float depth = In.Viewpos.z;//LinearizeDepth(In.curPos.z / In.curPos.w + depthBias.x, nearFar.x, nearFar.y);
	//�������ݐ�[�x�l�Z�o
    float2 coord = (In.curPos.xy / In.curPos.w) * float2(0.5f, -0.5f) + 0.5f;
    float screenDepth = ViewPosTexture.Sample(Sampler, coord).z; //LinearizeDepth(ViewPosTexture.Sample(Sampler, coord).w, nearFar.x, nearFar.y);
	//�[�x�̍��Z�o
    depth = distance(depth, screenDepth);
	//�[�x�̍���m_Length�ȉ��Ȃ瓧����
    //float a =1;
    if (depth <= softParticleArea)
    {
        depth /= softParticleArea;
        Out.a *= depth;
    //a =depth;
    }
    //Out = float4(a,a,a,1);
#endif

    return Out;
}
//���f���̃A���x�h�����̂܂܏o���V�F�[�_
float4 PSMain_SozaiNoAzi(PSInput In) : SV_Target0{
    return SozaiNoAziInner(In);
}
//��Z�ς݃A���t�@�ɕϊ������
float4 PSMain_SozaiNoAzi_ConvertToPMA(PSInput In) : SV_Target0{
    float4 color = SozaiNoAziInner(In);
	color.rgb *= color.a;
	return color;
}

#endif //DW_MODEL_FX