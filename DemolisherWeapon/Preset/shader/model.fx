/*!
 * @brief	���f���V�F�[�_�[�B
 */


/////////////////////////////////////////////////////////////
// Shader Resource View
/////////////////////////////////////////////////////////////
//�A���x�h�e�N�X�`���B
Texture2D<float4> albedoTexture : register(t0);	
//�{�[���s��
StructuredBuffer<float4x4> boneMatrix : register(t1);
StructuredBuffer<float4x4> boneMatrixOld : register(t2);

/////////////////////////////////////////////////////////////
// SamplerState
/////////////////////////////////////////////////////////////
sampler Sampler : register(s0);

/////////////////////////////////////////////////////////////
// �萔�o�b�t�@�B
/////////////////////////////////////////////////////////////
/*!
 * @brief	���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[�p�̒萔�o�b�t�@�B
 */
cbuffer VSPSCb : register(b0){
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;

	//�O��̍s��
	float4x4 mWorld_old;
	float4x4 mView_old;
	float4x4 mProj_old;
};

//�}�e���A���p�����[�^�[
cbuffer MaterialCb : register(b1) {
	float4 albedoScale;		//�A���x�h�ɂ�����X�P�[��
	float3 emissive;		//�G�~�b�V�u(���Ȕ���)
	int isLighting;			//���C�e�B���O���邩
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
    uint4  Indices  : BLENDINDICES0;		//���̒��_�Ɋ֘A�t������Ă���{�[���ԍ��Bx,y,z,w�̗v�f�ɓ����Ă���B4�{�[���X�L�j���O�B
    float4 Weights  : BLENDWEIGHT0;			//���̒��_�Ɋ֘A�t������Ă���{�[���ւ̃X�L���E�F�C�g�Bx,y,z,w�̗v�f�ɓ����Ă���B4�{�[���X�L�j���O�B
};

/*!
 * @brief	�s�N�Z���V�F�[�_�[�̓��́B
 */
struct PSInput{
	float4 Position 	: SV_POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float2 TexCoord 	: TEXCOORD0;
	float3 Viewpos		: TEXCOORD1;

	float4	curPos		: CUR_POSITION;//���ݍ��W
	float4	lastPos		: LAST_POSITION;//�ߋ����W
	bool isWorldMove	: IS_WORLD_BLUR;//���[���h��Ԃňړ����Ă��邩?
};

//Z�l�������݃s�N�Z���V�F�[�_�[�̓���
struct ZPSInput {
	float4 Position 	: SV_POSITION;
	float2 TexCoord 	: TEXCOORD0;
	float4 posInProj	: TEXCOORD1;
};

//���I�����N

//���[�V�����u���[�p����

//���_�V�F�[�_
struct CalcOldPosReturn {
	bool isWorldMove;
	float4 lastPos;
};
interface iBaseCalcOldPos {
	CalcOldPosReturn CalcOldPos(float4 pos, float4 Position, float3 posW);
	CalcOldPosReturn CalcOldPosSkin(float4 pos, float4 Position, float3 posW, uint4 Indices, float4 Weights);
};
class cCalcOldPos : iBaseCalcOldPos {
	CalcOldPosReturn CalcOldPos(float4 pos, float4 Position, float3 posW) {
		CalcOldPosReturn Out;

		float4 oldpos = mul(mWorld_old, Position);

		if (distance(posW, oldpos.xyz) > 0.0f) {
			Out.isWorldMove = true;
		}

		oldpos = mul(mView_old, oldpos);
		oldpos = mul(mProj_old, oldpos);

		if (oldpos.z >= 0.0f) {
			Out.lastPos = oldpos;
		}
		else {
			Out.lastPos = pos;
		}

		return Out;
	}
	CalcOldPosReturn CalcOldPosSkin(float4 pos, float4 Position, float3 posW, uint4 Indices, float4 Weights) {
		CalcOldPosReturn Out;
		
		float4x4 oldskinning = 0;
		float4 oldpos = 0;
		{
			float w = 0.0f;
			for (int i = 0; i < 3; i++)
			{
				oldskinning += boneMatrixOld[Indices[i]] * Weights[i];
				w += Weights[i];
			}
			oldskinning += boneMatrixOld[Indices[3]] * (1.0f - w);
			oldpos = mul(oldskinning, Position);
		}

		if (distance(posW, oldpos.xyz) > 0.0f) {
			Out.isWorldMove = true;
		}

		oldpos = mul(mView_old, oldpos);
		oldpos = mul(mProj_old, oldpos);

		if (oldpos.z >= 0.0f){
			Out.lastPos = oldpos;
		}
		else {
			Out.lastPos = pos;
		}

		return Out;
	}
};
class cNotCalcOldPos : iBaseCalcOldPos {
	CalcOldPosReturn CalcOldPos(float4 pos, float4 Position, float3 posW) {
		CalcOldPosReturn Out = (CalcOldPosReturn)0;
		return Out;
	}
	CalcOldPosReturn CalcOldPosSkin(float4 pos, float4 Position, float3 posW, uint4 Indices, float4 Weights) {
		CalcOldPosReturn Out = (CalcOldPosReturn)0;
		return Out;
	}
};
iBaseCalcOldPos g_calcOldPos;

//�s�N�Z���V�F�[�_
interface iBaseCalcVelocity {
	float2x4 CalcVelocity(float4 curPos, float4 lastPos, bool isWorldMove);
};
class cCalcVelocity : iBaseCalcVelocity {
	float2x4 CalcVelocity(float4 curPos, float4 lastPos, bool isWorldMove) {
		float2x4 Out = (float2x4)0;

		float2	current = curPos.xy / curPos.w;
		float2	last = lastPos.xy / lastPos.w;

		//if (In.curPos.z < 0.0f) { current *= -1.0f; }
		//if (In.lastPos.z < 0.0f) { last *= -1.0f;  }

		if (curPos.z < 0.0f || lastPos.z < 0.0f) {
			current *= 0.0f; last *= 0.0f;
		}

		current.xy *= float2(0.5f, -0.5f); current.xy += 0.5f;
		last.xy *= float2(0.5f, -0.5f); last.xy += 0.5f;

		Out[0].z = min(curPos.z, lastPos.z);
		Out[0].w = max(curPos.z, lastPos.z);

		if (isWorldMove) {
			Out[0].xy = current.xy - last.xy;
			Out[1].z = -1.0f;
			Out[1].w = -1.0f;
		}
		else {
			Out[1].xy = current.xy - last.xy;
			Out[1].z = min(curPos.z, lastPos.z);
			Out[1].w = max(curPos.z, lastPos.z);
		}

		return Out;
	}
};
class cNotCalcVelocity : iBaseCalcVelocity {
	float2x4 CalcVelocity(float4 curPos, float4 lastPos, bool isWorldMove) {
		float2x4 Out = (float2x4)0;
		Out[0].z = curPos.z;
		Out[0].w = curPos.z;
		Out[1].z = curPos.z;
		Out[1].w = curPos.z;//ps=1
		return Out;
	}
};
iBaseCalcVelocity g_calcVelocity;

/*!--------------------------------------------------------------------------------------
 * @brief	�X�L���Ȃ����f���p�̒��_�V�F�[�_�[�B
-------------------------------------------------------------------------------------- */
PSInput VSMain( VSInputNmTxVcTangent In ) 
{
	PSInput psInput = (PSInput)0;
	float4 pos = mul(mWorld, In.Position); float3 posW = pos.xyz;
	pos = mul(mView, pos); psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;
	psInput.Normal = normalize(mul(mWorld, In.Normal));
	psInput.Tangent = normalize(mul(mWorld, In.Tangent));

	psInput.curPos = pos;

	//�x���V�e�B�}�b�v�p���
	CalcOldPosReturn ReturnOld = g_calcOldPos.CalcOldPos(pos, In.Position, posW);
	psInput.isWorldMove = ReturnOld.isWorldMove;
	psInput.lastPos = ReturnOld.lastPos;

	return psInput;
}
//Z�l�������ݗp
ZPSInput VSMain_RenderZ(VSInputNmTxVcTangent In)
{
	ZPSInput psInput = (ZPSInput)0;
	float4 pos = mul(mWorld, In.Position);
	pos = mul(mView, pos);
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;

	return psInput;
}

/*!--------------------------------------------------------------------------------------
 * @brief	�X�L�����胂�f���p�̒��_�V�F�[�_�[�B
 * �S�Ă̒��_�ɑ΂��Ă��̃V�F�[�_�[���Ă΂��B
 * In��1�̒��_�f�[�^�BVSInputNmTxWeights�����Ă݂悤�B
-------------------------------------------------------------------------------------- */
PSInput VSMainSkin( VSInputNmTxWeights In ) 
{
	PSInput psInput = (PSInput)0;
	///////////////////////////////////////////////////
	//��������X�L�j���O���s���Ă���ӏ��B
	//�X�L���s����v�Z�B
	///////////////////////////////////////////////////
	float4x4 skinning = 0;	
	float4 pos = 0;
	{
	
		float w = 0.0f;
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
	  	//���_���W�ɃX�L���s�����Z���āA���_�����[���h��Ԃɕϊ��B
		//mul�͏�Z���߁B
	    pos = mul(skinning, In.Position);
	}
	float3 posW = pos.xyz;
	psInput.Normal = normalize( mul(skinning, In.Normal) );
	psInput.Tangent = normalize( mul(skinning, In.Tangent) );
	
	pos = mul(mView, pos);  psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;

	psInput.curPos = pos;

	//�x���V�e�B�}�b�v�p���
	CalcOldPosReturn ReturnOld = g_calcOldPos.CalcOldPosSkin(pos, In.Position, posW, In.Indices, In.Weights);
	psInput.isWorldMove = ReturnOld.isWorldMove;
	psInput.lastPos = ReturnOld.lastPos;

    return psInput;
}
//Z�l�������ݗp
ZPSInput VSMainSkin_RenderZ(VSInputNmTxWeights In)
{
	ZPSInput psInput = (ZPSInput)0;
	///////////////////////////////////////////////////
	//��������X�L�j���O���s���Ă���ӏ��B
	//�X�L���s����v�Z�B
	///////////////////////////////////////////////////
	float4x4 skinning = 0;
	float4 pos = 0;
	{

		float w = 0.0f;
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
		//���_���W�ɃX�L���s�����Z���āA���_�����[���h��Ԃɕϊ��B
		//mul�͏�Z���߁B
		pos = mul(skinning, In.Position);
	}

	pos = mul(mView, pos);
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;
	return psInput;
}

//G-Buffer�o��
struct PSOutput_RenderGBuffer {
	float4 albedo		: SV_Target0;		//�A���x�h
	float3 normal		: SV_Target1;		//�@��
	float4 viewpos		: SV_Target2;		//�r���[���W
	float4 velocity		: SV_Target3;		//���x
	float4 velocityPS	: SV_Target4;		//���x(�s�N�Z���V�F�[�_)
	float4 lightingParam: SV_Target5;		//���C�e�B���O�p�p�����[�^�[
};
PSOutput_RenderGBuffer PSMain_RenderGBuffer(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	//�A���x�h
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord) * albedoScale;

	//���e�X�g
	if (Out.albedo.a > 0.5f) { 
		Out.albedo.a = 1.0f;//����������
	}
	else {
		discard;
	}

	//�@��
	Out.normal = In.Normal;

	//�r���[���W
	Out.viewpos = float4(In.Viewpos, In.curPos.z / In.curPos.w);// In.curPos.z);

	//���C�e�B���O�p�p�����[�^�[
	Out.lightingParam.rgb = emissive;//�G�~�b�V�u(���Ȕ���)
	Out.lightingParam.a = isLighting;//���C�e�B���O���邩

	//���x
	float2x4 velocity = g_calcVelocity.CalcVelocity(In.curPos, In.lastPos, In.isWorldMove);
	Out.velocity = velocity[0];
	Out.velocityPS = velocity[1];

	return Out;
}

//Z�l�o��
float4 PSMain_RenderZ(ZPSInput In) : SV_Target0
{
	//�A���x�h
	float alpha = albedoTexture.Sample(Sampler, In.TexCoord).a * albedoScale.a;

	//���e�X�g
	if (alpha > 0.5f) {
	}
	else {
		discard;
	}

	float z = In.posInProj.z / In.posInProj.w;
	return z;
}