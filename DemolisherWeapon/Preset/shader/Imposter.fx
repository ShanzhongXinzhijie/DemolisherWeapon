#include"model.fx"

#if defined(INSTANCING)
//�C���X�^���V���O�p�e�N�X�`���C���f�b�N�X
StructuredBuffer<int2> InstancingImposterTextureIndex : register(t7);
#endif

///////////////////////////////////////////////////////////////////////
// �N�H�[�^�j�I���̌v�Z												 //
// https://gist.github.com/mattatz/40a91588d5fb38240403f198a938a593  //
///////////////////////////////////////////////////////////////////////
static const float PI = 3.14159265359f;
static const float PI2 = PI * 2.0f;
// Quaternion multiplication
// http://mathworld.wolfram.com/Quaternion.html
float4 qmul(float4 q1, float4 q2)
{
	return float4(
		q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
		q1.w * q2.w - dot(q1.xyz, q2.xyz)
		);
}
// Vector rotation with a quaternion
// http://mathworld.wolfram.com/Quaternion.html
float3 rotate_vector(float3 v, float4 r)
{
	float4 r_c = r * float4(-1, -1, -1, 1);
	return qmul(r, qmul(float4(v, 0), r_c)).xyz;
}
// A given angle of rotation about a given axis
float4 rotate_angle_axis(float angle, float3 axis)
{
	float sn = sin(angle * 0.5);
	float cs = cos(angle * 0.5);
	return float4(axis * sn, cs);
}

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

//�C���|�X�^�[�̌v�Z
void CalcImposter(out int2 out_index, inout float3 inout_pos
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

		//�J���������Ƀ��f���T�C�Y�����W���炷
		//�����܂�h�~
		//TODO ���������v���O�������ł��
	//#if !defined(SHADOW)
	//		polyDir *= scale * texture.GetDirectionOfCameraSize(index_x, index_y);
	//#endif

		//TODO �o�C�A�X�s��

		//��]		
		float r = -out_index.y * -(PI / (imposterPartNum.y - 1)) + PI * 0.5f;
		float sinr = sin(r), cosr = cos(r);
		float3x3 rotX = {1.0f,0.0f,0.0f,
						 0.0f,cosr,sinr,
						 0.0f,-sinr,cosr
						};
		r = -out_index.x * -(PI2 / (imposterPartNum.x - 1)) + PI2;
		sinr = sin(r), cosr = cos(r);
		float3x3 rotY = {cosr,sinr,0.0f, //��Z����]
						 -sinr,cosr,0.0f,
						 0.0f,0.0f,1.0f
						};
		//{cosr, 0.0f, -sinr,
		//	0.0f, 1.0f, 0.0f,
		//	sinr, 0.0f, cosr
		//};
		inout_pos = mul(inout_pos, rotX);
		inout_pos = mul(inout_pos, rotY);
		
/*		
		float4 rotX = rotate_angle_axis(-out_index.y * -(PI / (imposterPartNum.y - 1)) + PI * 0.5f, float3(1.0f, 0.0f, 0.0f));
		float4 rotY = rotate_angle_axis(-out_index.x * -(PI2 / (imposterPartNum.x - 1)) + PI2, float3(0.0f, 0.0f, 1.0f));
		rotX = qmul(rotY, rotX);
		inout_pos = rotate_vector(inout_pos, rotX);
*/
		//�Ԃ�
		//position_return = pos + polyDir;
		//rotation_return = rot;
		//TODO
		//scale_return = scale * texture.GetModelSize()*2.0f;
}

//���_�V�F�[�_(�ʏ�)
PSInput VSMain_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	int2 index;
	CalcImposter(index, In.Position.xyz
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	PSInput psInput = VSMain(In
#if defined(INSTANCING)
							, instanceID
#endif 
							);

	psInput.imposterIndex = index;

	return psInput;
}
//���_�V�F�[�_(�[�x�l)
ZPSInput VSMain_RenderZ_Imposter(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	int2 index;
	CalcImposter(index, In.Position.xyz
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	ZPSInput psInput = VSMain_RenderZ(In
#if defined(INSTANCING)
		, instanceID
#endif 
		);

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
#if defined(INSTANCING)
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
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
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
#else
	In.TexCoord.x += (1.0f / imposterPartNum.x) * In.imposterIndex.x;
	In.TexCoord.y += (1.0f / imposterPartNum.y) * In.imposterIndex.y;
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