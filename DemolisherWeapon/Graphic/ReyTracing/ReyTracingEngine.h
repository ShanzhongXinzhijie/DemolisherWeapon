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

	enum EnReyTracingHitGroup {
		enHitGroup_Undef = -1,
		enHitGroup_PBRCameraRay,	//PBR�}�e���A���ɃJ�������C���Փ˂���Ƃ��̃q�b�g�O���[�v�B
		enHitGroup_PBRShadowRay,	//PBR�}�e���A���ɃV���h�E���C���Փ˂���Ƃ��̃q�b�g�O���[�v�B
		enHitGroup_Num,			//�q�b�g�O���[�v�̐��B
	};

	struct ReyTracingInstanceData {
		D3D12_RAYTRACING_GEOMETRY_DESC geometoryDesc;	//�W�I���g�����B
		StructuredBufferInnerDX12 m_vertexBufferRWSB;			//���_�o�b�t�@�B
		StructuredBufferInnerDX12 m_indexBufferRWSB;			//�C���f�b�N�X�o�b�t�@�B
		MaterialData* m_material = nullptr;			//�}�e���A���B		
	};

	struct AccelerationStructureBuffers {
		ID3D12Resource* pScratch = nullptr;
		ID3D12Resource* pResult = nullptr;
		ID3D12Resource* pInstanceDesc = nullptr;
	};

	const int MAX_TRACE_RECURSION_DEPTH = 4;	//���C�g���[�X�̍ċA�Ăяo���̍ő吔�B
												//���ꂪTraceRay���ċA�I�ɌĂяo����ő吔�ł��B
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
		eShader_ShadowMiss,			//�V���h�E���C���ǂ��ɂ��Ԃ���Ȃ��������ɌĂ΂��V�F�[�_�[�B
		eShader_Num,				//�V�F�[�_�[�̐��B
	};

	//�V�F�[�_�[�̃J�e�S���B
	enum EShaderCategory {
		eShaderCategory_RayGenerator,	//���C�𐶐�����V�F�[�_�[�B
		eShaderCategory_Miss,			//�~�X�V�F�[�_�[�B
		eShaderCategory_ClosestHit,		//�����Ƃ��߂��|���S���ƃ��C�����������Ƃ��ɌĂ΂��V�F�[�_�[�B
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
		{ L"shadowMiss",	eLocalRootSignature_Empty,			eShaderCategory_Miss,			eHitGroup_Undef },
	};
	static_assert(ARRAYSIZE(shaderDatas) == eShader_Num, "shaderDatas arraySize is invalid!! shaderDatas arraySize must be equal to eShader_Num");

	//�q�b�g�O���[�v�\���́B
	struct SHitGroup {
		const wchar_t* name;				//�q�b�g�O���[�v�̖��O�B
		const wchar_t* chsHitShaderName;	//�ł��߂��|���S���Ƀq�b�g�����Ƃ��ɌĂ΂��V�F�[�_�[�̖��O�B
		const wchar_t* anyHitShaderName;	//any shader???
	};
	const SHitGroup hitGroups[] = {
		{ L"HitGroup",			shaderDatas[eShader_PBRChs].entryPointName,	nullptr },
		{ L"ShadowHitGroup",	shaderDatas[eShader_ShadowChs].entryPointName, nullptr },
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
		eNum,								//SRV�̐��B
		eNumRayGenerationSRV = eEndRayGenerationSRV - eStartRayGenerationSRV,//���C�W�F�l���[�V�����V�F�[�_�[�Ŏg�p����SRV�̐��B
	};

	/// <summary>
	/// �q�b�g�V�F�[�_�[�̃f�B�X�N���v�^�e�[�u��
	/// </summary>
	enum EHitShaderDescriptorTable {
		eHitShaderDescriptorTable_SRV_CBV,	//SRV��CBV
		eHitShaderDescriptorTable_Sampler,	//�T���v��
		eHitShaderDescriptorTable_Num       //�e�[�u���̐��B
	};

	class BLASBuffer {
	public:
		/// <summary>
		/// �������B
		/// </summary>
		/// <param name="commandList"></param>
		/// <param name="instance"></param>
		void Init(ID3D12GraphicsCommandList4* commandList, const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instance);

		/// /// <summary>
		/// BLASBuffer�̃��X�g���擾�B
		/// </summary>
		/// <returns></returns>
		const std::vector<AccelerationStructureBuffers>& Get() const
		{
			return m_bottomLevelASBuffers;
		}

	private:
		std::vector<AccelerationStructureBuffers> m_bottomLevelASBuffers;
	};

	class TLASBuffer {// : public IShaderResource {
	public:
		/// <summary>
		/// TLAS���\�z�B
		/// </summary>
		/// <param name="rc"></param>
		/// <param name="instances"></param>
		void Init(
			ID3D12GraphicsCommandList4* commandList,
			const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instances,
			const std::vector<AccelerationStructureBuffers>& bottomLevelASBuffers
		);

		/// <summary>
		/// SRV�ɓo�^�B
		/// </summary>
		void CreateShaderResourceView();

		/// <summary>
		/// VRAM��̉��z�A�h���X���擾�B
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() const{
			return m_GPUdescriptorHandle;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const {
			return m_CPUdescriptorHandle;
		}

	private:
		AccelerationStructureBuffers m_topLevelASBuffers;

		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUdescriptorHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUdescriptorHandle;
	};

	class ReyTracingWorld;

	/// <summary>
	/// �萔�o�b�t�@
	/// </summary>
	struct ReyTracingCBStructure {
		CMatrix mRot;	//��]�s��
		CVector3 pos;	//���_�B
		float aspect;	//�A�X�y�N�g��B
		float fFar;		//�����ʁB
		float fNear;	//�ߕ��ʁB
	};

	/// <summary>
	/// ���C�g���̃f�B�X�N���v�^�q�[�v
	/// </summary>
	class ReyTracingDescriptorHeap {
	public:
		void Init(ReyTracingWorld& world, ConstantBufferDx12<ReyTracingCBStructure>& cb, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle);

		ID3D12DescriptorHeap* GetSRVHeap()const {
			return m_srvsDescriptorHeap.Get();
		}
		ID3D12DescriptorHeap* GetSamplerHeap()const {
			return m_samplerDescriptorHeap.Get();
		}

		/// <summary>
		/// SRV�f�X�N���v�^�̃T�C�Y
		/// </summary>
		/// <returns></returns>
		UINT GetSrvsDescriptorSize()const {
			return m_srvsDescriptorSize;
		}

		/// <summary>
		/// UAV�f�X�N���v�^�̊J�n�ʒu
		/// </summary>
		/// <returns></returns>
		int GetOffsetUAVDescriptorFromTableStart()const {
			return m_uavStartNum;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvsDescriptorHeap;
		UINT m_srvsDescriptorSize = 0;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerDescriptorHeap;
		UINT m_samplerDescriptorSize = 0;

		int m_uavStartNum = 0;
	};

	/// <summary>
	/// ���C�g���p�̃p�C�v���C���X�e�[�g�I�u�W�F�N�g�B
	/// </summary>
	class ReyTracingPSO {
	private:
		/// <summary>
		/// ���[�g�V�O�l�`����`�B
		/// </summary>
		struct RootSignatureDesc
		{
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			std::vector<D3D12_DESCRIPTOR_RANGE> range;
			std::vector<D3D12_ROOT_PARAMETER> rootParams;
		};

	public:
		/// <summary>
		/// �p�C�v���C���X�e�[�g�I�u�W�F�N�g���������B
		/// </summary>
		void Init(const ReyTracingDescriptorHeap* DH);

		void QueryInterface(Microsoft::WRL::ComPtr<ID3D12StateObjectProperties>& props) const
		{
			m_pipelineState.As(&props);
			//m_pipelineState->QueryInterface(&props);
		}

		/// <summary>
		/// �O���[�o�����[�g�V�O�l�`�����擾�B
		/// </summary>
		/// <returns></returns>
		ID3D12RootSignature* GetGlobalRootSignature()
		{
			return m_emptyRootSignature.Get();
		}

		ID3D12StateObject* Get()
		{
			return m_pipelineState.Get();
		}

	private:
		RootSignatureDesc CreateRayGenRootSignatureDesc();
		RootSignatureDesc CreatePBRMatterialHitRootSignatureDesc();

	private:
		const ReyTracingDescriptorHeap* m_descriptorHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12StateObject> m_pipelineState;					//�p�C�v���C���X�e�[�g
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_emptyRootSignature;
	};

	/// <summary>
	/// ���C�g�����[���g�J
	/// </summary>
	class ReyTracingWorld
	{
	public:
		/// <summary>
		/// �W�I���g����o�^�B
		/// </summary>
		/// <param name="model">���f��</param>
		void RegistGeometry(CModel& model);
		/// <summary>
		/// �W�I���g���̓o�^���m��B
		/// (BLAS��TLAS�𐶐�)
		/// </summary>
		void CommitRegistGeometry(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// ���C�g�����[���h�̃C���X�^���X�ɑ΂��ăN�G�����s���B
		/// </summary>
		/// <param name="queryFunc"></param>
		void QueryInstances(std::function<void(ReyTracingInstanceData&)> queryFunc) const
		{
			for (auto& instance : m_instances) {
				queryFunc(*instance);
			}
		}

		/// <summary>
		/// �C���X�^���X�̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumInstance() const
		{
			return (int)m_instances.size();
		}

		/// <summary>
		/// TLAS�擾
		/// </summary>
		/// <returns></returns>
		TLASBuffer& GetTLASBuffer() {
			return m_topLevelASBuffers;
		}

	private:
		std::vector<std::unique_ptr<ReyTracingInstanceData>> m_instances;
		BLASBuffer m_blasBuffer;
		TLASBuffer m_topLevelASBuffers;
	};

	/// <summary>
	/// �V�F�[�_�[�e�[�u��
	/// </summary>
	/// <remark>
	/// �V�F�[�_�[�e�[�u���̓��C�̃f�B�X�p�b�`�Ŏg�p�����A
	/// �V�F�[�_�[�⃊�\�[�X�̃f�B�X�N���v�^�q�[�v�̃A�h���X�Ȃǂ��o�^����Ă���e�[�u���ł��B
	/// </remark>
	class ShaderTable {
	public:
		/// <summary>
		/// �V�F�[�_�[�e�[�u�����������B
		/// </summary>
		void Init(
			const ReyTracingWorld& world,
			const ReyTracingPSO& pso,
			const ReyTracingDescriptorHeap& descriptorHeaps
		);

		/// <summary>
		/// �V�F�[�_�[�e�[�u����GPU��̉��z�A�h���X���擾�B
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_shaderTable->GetGPUVirtualAddress();
		}
		/// <summary>
		/// �V�F�[�_�[�e�[�u���ɋL������Ă���f�[�^�̂P�v�f�̃T�C�Y���擾�B
		/// </summary>
		/// <returns></returns>
		uint32_t GetShaderTableEntrySize() const
		{
			return m_shaderTableEntrySize;
		}
		/// <summary>
		/// ���C�W�F�l���[�V�����V�F�[�_�[�̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumRayGenShader() const
		{
			return m_numRayGenShader;
		}
		/// <summary>
		/// �~�X�V�F�[�_�[�̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumMissShader() const
		{
			return m_numMissShader;
		}
		/// <summary>
		/// �V�F�[�_�[�e�[�u���ɓo�^����Ă���q�b�g�V�F�[�_�[�̐����擾�B
		/// </summary>
		/// <remark>
		/// �q�b�g�V�F�[�_�[�̐��̓C���X�^���X�̐��Ɠ����ɂȂ�܂��B
		/// </remark>
		/// <returns></returns>
		int GetNumHitShader() const
		{
			return m_numHitShader;
		}
	private:
		/// <summary>
		/// ���C�W�F�l���[�V�����V�F�[�_�[�A�~�X�V�F�[�_�[�A�q�b�g�V�F�[�_�[�̐����J�E���g����B
		/// </summary>
		void CountupNumGeyGenAndMissAndHitShader();
		/// <summary>
		/// �V�F�[�_�[�e�[�u����1�v�f�̃T�C�Y���v�Z����B
		/// </summary>
		void CalcShaderTableEntrySize();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_shaderTable;			//�V�F�[�_�[�e�[�u���B
		uint32_t m_shaderTableEntrySize = 0;
		int m_numRayGenShader = 0;
		int m_numMissShader = 0;
		int m_numHitShader = 0;
	};

	/// <summary>
	/// ���C�g���[�V���O�G���W��
	/// </summary>
	class RayTracingEngine
	{
	public:
		/// <summary>
		/// ���C�g���[�V���O���f�B�X�p�b�`�B
		/// </summary>
		/// <param name="rc">�����_�����O�R���e�L�X�g</param>
		void Dispatch(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// �W�I���g����o�^�B
		/// </summary>
		/// <param name="model">���f��</param>
		void RegistGeometry(CModel& model)
		{
			//���C�g�����[���h�ɃW�I���g����o�^�B
			m_world.RegistGeometry(model);
		}

		/// <summary>
		/// �W�I���g���̓o�^���m��B
		/// </summary>
		void CommitRegistGeometry(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// �萔�o�b�t�@�擾
		/// </summary>
		/// <returns></returns>
		ConstantBufferDx12<ReyTracingCBStructure>& GetCB() {
			return m_rayGenerationCB;
		}

	private:
		/// <summary>
		/// �V�F�[�_�[���\�[�X���쐬�B
		/// </summary>
		void CreateShaderResources();

	private:
		ReyTracingWorld m_world;
		ReyTracingPSO m_pipelineStateObject;
		ShaderTable m_shaderTable;
		ReyTracingDescriptorHeap m_descriptorHeap;

		ConstantBufferDx12<ReyTracingCBStructure> m_rayGenerationCB;//�萔�o�b�t�@
		ReyTracingCBStructure m_cbStructure;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_raytracingOutput;//�o�̓o�b�t�@
		D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;//�o�̓o�b�t�@��GPU�f�B�X�N���v�^�n���h��
		D3D12_CPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVCpuDescriptor;//�o�̓o�b�t�@��CPU�f�B�X�N���v�^�n���h��
	};

}
