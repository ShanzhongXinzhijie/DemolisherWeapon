#pragma once

namespace DemolisherWeapon {
	inline constexpr D3D12_HEAP_PROPERTIES kDefaultHeapProps =
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};
	inline constexpr D3D12_HEAP_PROPERTIES kUploadHeapProps =
	{
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0,
	};

	/// <summary>
	/// AccelerationStructure
	/// </summary>
	struct AccelerationStructureBuffers {
		Microsoft::WRL::ComPtr<ID3D12Resource> pScratch;
		Microsoft::WRL::ComPtr<ID3D12Resource> pResult;
		Microsoft::WRL::ComPtr<ID3D12Resource> pInstanceDesc;
	};

	/// <summary>
	/// ���C�g���p�W�I���g�����
	/// </summary>
	struct ReyTracingGeometoryData {
		D3D12_RAYTRACING_GEOMETRY_DESC m_geometoryDesc;	//�W�I���g�����
		StructuredBufferInnerDX12 m_vertexBufferRWSB;	//���_�o�b�t�@
		StructuredBufferInnerDX12 m_indexBufferRWSB;	//�C���f�b�N�X�o�b�t�@

		//AccelerationStructureBuffers m_BLAS;
		ID3D12Resource* m_pResultBLAS = nullptr;
	};
	/// <summary>
	/// ���C�g���p�C���X�^���X���
	/// </summary>
	struct ReyTracingInstanceData {
		const ReyTracingGeometoryData* m_geometory = nullptr;	//�W�I���g�����
		const MaterialSetting* m_material = nullptr;			//�}�e���A��
		const CMatrix* m_worldMatrix = nullptr;					//���[���h�s��
	};

	//���C�g���[�X�̍ċA�Ăяo���̍ő吔�B
	//���ꂪTraceRay���ċA�I�ɌĂяo����ő吔�ł��B
	inline constexpr int MAX_TRACE_RECURSION_DEPTH = 4;

	//���[�J�����[�g�V�O�l�`��
	enum ELocalRootSignature {
		eLocalRootSignature_Empty,				//��̃��[�J�����[�g�V�O�l�`���B
		eLocalRootSignature_Raygen,				//���C�����V�F�[�_�[�p�̃��[�J�����[�g�V�O�l�`���B
		eLocalRootSignature_PBRMaterialHit,		//PBR�}�e���A���Ƀq�b�g�����Ƃ��̃��[�J�����[�g�V�O�l�`���B
	};

	//�V�F�[�_�[
	enum EShader {
		eShader_Raygeneration,		//�J�������C�𐶐�����V�F�[�_�[�B
		eShader_Miss,				//�J�������C���ǂ��ɂ��Ԃ���Ȃ��������ɌĂ΂��V�F�[�_�[�B
		eShader_PBRChs,				//�����Ƃ��߂��|���S���ƃJ�������C�����������Ƃ��ɌĂ΂��V�F�[�_�[�B
		eShader_ShadowChs,			//�����Ƃ��߂��|���S���ƃV���h�E���C�����������Ƃ��ɌĂ΂��V�F�[�_�[�B
		eShader_ShadowAny,
		eShader_ShadowMiss,			//�V���h�E���C���ǂ��ɂ��Ԃ���Ȃ��������ɌĂ΂��V�F�[�_�[�B
		eShader_Num,				//�V�F�[�_�[�̐��B
	};
	//�V�F�[�_�[�̃J�e�S���B
	enum EShaderCategory {
		eShaderCategory_RayGenerator,	//���C�𐶐�����V�F�[�_�[�B
		eShaderCategory_Miss,			//�~�X�V�F�[�_�[�B
		eShaderCategory_ClosestHit,		//�����Ƃ��߂��|���S���ƃ��C�����������Ƃ��ɌĂ΂��V�F�[�_�[�B
		eShaderCategory_AnyHit,			//Any hit shader
	};
	//�q�b�g�O���[�v�B
	enum EHitGroup {
		eHitGroup_Undef = -1,
		eHitGroup_PBRCameraRay,	//PBR�}�e���A���ɃJ�������C���Փ˂���Ƃ��̃q�b�g�O���[�v�B
		eHitGroup_PBRShadowRay,	//PBR�}�e���A���ɃV���h�E���C���Փ˂���Ƃ��̃q�b�g�O���[�v�B
		eHitGroup_Num,			//�q�b�g�O���[�v�̐��B
	};
	//�V�F�[�_�[�f�[�^�\���́B
	struct ShaderData {
		const wchar_t* entryPointName;				//�G���g���[�|�C���g�̖��O�B
		ELocalRootSignature useLocalRootSignature;	//�g�p���郍�[�J�����[�g�V�O�l�`���B
		EShaderCategory category;					//�J�e�S���[�B
		EHitGroup hitgroup;							//�q�b�g�O���[�v�B
													//�J�e�S����eShaderCategory_Miss�AeShaderCategory_RayGenerator�̏ꍇ�A���̃p�����[�^�[�͖�������܂��B
	};
	constexpr inline ShaderData shaderDatas[] = {
		//entryPointName	useLocalRootSignature				category						hitgroup
		{ L"rayGen",		eLocalRootSignature_Raygen,			eShaderCategory_RayGenerator,	eHitGroup_Undef },
		{ L"miss",			eLocalRootSignature_Empty,			eShaderCategory_Miss,			eHitGroup_Undef },
		{ L"chs",			eLocalRootSignature_PBRMaterialHit,	eShaderCategory_ClosestHit,		eHitGroup_PBRCameraRay },
		{ L"shadowChs",		eLocalRootSignature_PBRMaterialHit,	eShaderCategory_ClosestHit,		eHitGroup_PBRShadowRay },
		{ L"shadowAny",		eLocalRootSignature_PBRMaterialHit,	eShaderCategory_AnyHit,			eHitGroup_PBRShadowRay },
		{ L"shadowMiss",	eLocalRootSignature_Empty,			eShaderCategory_Miss,			eHitGroup_Undef },
	};
	static_assert(ARRAYSIZE(shaderDatas) == eShader_Num, "shaderDatas arraySize is invalid!! shaderDatas arraySize must be equal to eShader_Num");

	//�q�b�g�O���[�v�\���́B
	struct SHitGroup {
		const wchar_t* name;				//�q�b�g�O���[�v�̖��O�B
		const wchar_t* chsHitShaderName;	//�ł��߂��|���S���Ƀq�b�g�����Ƃ��ɌĂ΂��V�F�[�_�[�̖��O�B
		const wchar_t* anyHitShaderName;	//any hit shader
	};
	constexpr inline  SHitGroup hitGroups[] = {
		{ L"HitGroup",			shaderDatas[eShader_PBRChs].entryPointName,	shaderDatas[eShader_ShadowAny].entryPointName },
		{ L"ShadowHitGroup",	shaderDatas[eShader_ShadowChs].entryPointName, shaderDatas[eShader_ShadowAny].entryPointName },
	};
	static_assert(ARRAYSIZE(hitGroups) == eHitGroup_Num, "hitGroups arraySize is invalid!! hitGroups arraySize must be equal to eHitGoup_Num");

	/// <summary>
	/// �V�F�[�_�[�e�[�u���ɓo�^����Ă���SRV��1�v�f
	/// </summary>
	/// <remarks>
	/// ���̗񋓎q�̕��т�t���W�X�^�̔ԍ��ɂȂ�܂��B
	/// �V�F�[�_�[�e�[�u���ɂ͊e�C���X�^���X���ƂɃV�F�[�_�[���\�[�X�̃f�B�X�N���v�^���o�^����Ă��܂��B
	/// ���̗񋓎q���e�C���X�^���X�Ɋ��蓖�Ă��Ă���V�F�[�_�[���\�[�X��\���Ă��܂��B
	/// </remarks>
	enum class ESRV_OneEntry {
		eStartRayGenerationSRV,				//���C�W�F�l���[�V�����V�F�[�_�[�ŗ��p����SRV�̊J�n�ԍ��B
		eTLAS = eStartRayGenerationSRV,		//TLAS
		eEndRayGenerationSRV,				//���C�W�F�l���[�V�����Ŏg�p�����SRV�̐��B
		eAlbedoMap = eEndRayGenerationSRV,	//�A���x�h�}�b�v�B
		eNormalMap,							//�@���}�b�v�B
		eSpecularMap,						//�X�y�L�����}�b�v�B
		eReflectionMap,						//���t���N�V�����}�b�v�B
		eRefractionMap,						//���܃}�b�v�B
		eVertexBuffer,						//���_�o�b�t�@�B
		eIndexBuffer,						//�C���f�b�N�X�o�b�t�@�B
		eWorldMatrixs,						//���[���h�s��
		eNum,								//SRV�̐��B
		eNumRayGenerationSRV = eEndRayGenerationSRV - eStartRayGenerationSRV,//���C�W�F�l���[�V�����V�F�[�_�[�Ŏg�p����SRV�̐��B
	};

	enum EHitShaderDescriptorTable {
		eHitShaderDescriptorTable_SRV_CBV,	//SRV��CBV
		eHitShaderDescriptorTable_Sampler,	//�T���v��
		eHitShaderDescriptorTable_Num       //�e�[�u���̐��B
	};

	/// <summary>
	/// �萔�o�b�t�@
	/// </summary>
	struct ReyTracingCBStructure {
		CMatrix mRot;	//��]�s��
		CVector3 pos;	//���_�B
		float fFar;		//�����ʁB
		float fNear;	//�ߕ��ʁB
		float fov;      //����p
	};
}