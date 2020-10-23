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
		enHitGroup_PBRCameraRay,	//PBRマテリアルにカメラレイが衝突するときのヒットグループ。
		enHitGroup_PBRShadowRay,	//PBRマテリアルにシャドウレイが衝突するときのヒットグループ。
		enHitGroup_Num,			//ヒットグループの数。
	};

	struct ReyTracingInstanceData {
		D3D12_RAYTRACING_GEOMETRY_DESC geometoryDesc;	//ジオメトリ情報。
		//RWStructuredBuffer m_vertexBufferRWSB;			//頂点バッファ。
		//RWStructuredBuffer m_indexBufferRWSB;			//インデックスバッファ。
		//Material* m_material = nullptr;					//マテリアル。		
	};

	struct AccelerationStructureBuffers {
		ID3D12Resource* pScratch = nullptr;
		ID3D12Resource* pResult = nullptr;
		ID3D12Resource* pInstanceDesc = nullptr;
	};

	const int MAX_TRACE_RECURSION_DEPTH = 4;	//レイトレースの再帰呼び出しの最大数。
												//これがTraceRayを再帰的に呼び出せる最大数です。
	//ローカルルートシグネチャ
	enum ELocalRootSignature {
		eLocalRootSignature_Empty,				//空のローカルルートシグネチャ。
		eLocalRootSignature_Raygen,				//レイ生成シェーダー用のローカルルートシグネチャ。
		eLocalRootSignature_PBRMaterialHit,		//PBRマテリアルにヒットしたときのローカルルートシグネチャ。
	};

	//シェーダー
	enum EShader {
		eShader_Raygeneration,		//カメラレイを生成するシェーダー。
		eShader_Miss,				//カメラレイがどこにもぶつからなかった時に呼ばれるシェーダー。
		eShader_PBRChs,				//もっとも近いポリゴンとカメラレイが交差したときに呼ばれるシェーダー。
		eShader_ShadowChs,			//もっとも近いポリゴンとシャドウレイが交差したときに呼ばれるシェーダー。
		eShader_ShadowMiss,			//シャドウレイがどこにもぶつからなかった時に呼ばれるシェーダー。
		eShader_Num,				//シェーダーの数。
	};

	//シェーダーのカテゴリ。
	enum EShaderCategory {
		eShaderCategory_RayGenerator,	//レイを生成するシェーダー。
		eShaderCategory_Miss,			//ミスシェーダー。
		eShaderCategory_ClosestHit,		//もっとも近いポリゴンとレイが交差したときに呼ばれるシェーダー。
	};

	//ヒットグループ。
	enum EHitGroup {
		eHitGroup_Undef = -1,
		eHitGroup_PBRCameraRay,	//PBRマテリアルにカメラレイが衝突するときのヒットグループ。
		eHitGroup_PBRShadowRay,	//PBRマテリアルにシャドウレイが衝突するときのヒットグループ。
		eHitGroup_Num,			//ヒットグループの数。
	};

	//シェーダーデータ構造体。
	struct ShaderData {
		const wchar_t* entryPointName;				//エントリーポイントの名前。
		ELocalRootSignature useLocalRootSignature;	//使用するローカルルートシグネチャ。
		EShaderCategory category;					//カテゴリー。
		EHitGroup hitgroup;							//ヒットグループ。
													//カテゴリがeShaderCategory_Miss、eShaderCategory_RayGeneratorの場合、このパラメーターは無視されます。
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

	//ヒットグループ構造体。
	struct SHitGroup {
		const wchar_t* name;				//ヒットグループの名前。
		const wchar_t* chsHitShaderName;	//最も近いポリゴンにヒットしたときに呼ばれるシェーダーの名前。
		const wchar_t* anyHitShaderName;	//any shader???
	};
	const SHitGroup hitGroups[] = {
		{ L"HitGroup",			shaderDatas[eShader_PBRChs].entryPointName,	nullptr },
		{ L"ShadowHitGroup",	shaderDatas[eShader_ShadowChs].entryPointName, nullptr },
	};
	static_assert(ARRAYSIZE(hitGroups) == eHitGroup_Num, "hitGroups arraySize is invalid!! hitGroups arraySize must be equal to eHitGoup_Num");

	/// <summary>
	/// シェーダーテーブルに登録されているSRVの1要素
	/// </summary>
	/// <remarks>
	/// この列挙子の並びがtレジスタの番号になります。
	/// シェーダーテーブルには各インスタンスごとにシェーダーリソースのディスクリプタが登録されています。
	/// この列挙子が各インスタンスに割り当てられているシェーダーリソースを表しています。
	/// </remarks>
	enum class ESRV_OneEntry {
		eStartRayGenerationSRV,				//レイジェネレーションシェーダーで利用するSRVの開始番号。
		eTLAS = eStartRayGenerationSRV,		//TLAS
		eEndRayGenerationSRV,				//レイジェネレーションで使用されるSRVの数。
		eAlbedoMap = eEndRayGenerationSRV,	//アルベドマップ。
		eNormalMap,							//法線マップ。
		eSpecularMap,						//スペキュラマップ。
		eReflectionMap,						//リフレクションマップ。
		eRefractionMap,						//屈折マップ。
		eVertexBuffer,						//頂点バッファ。
		eIndexBuffer,						//インデックスバッファ。
		eNum,								//SRVの数。
		eNumRayGenerationSRV = eEndRayGenerationSRV - eStartRayGenerationSRV,//レイジェネレーションシェーダーで使用するSRVの数。
	};

	/// <summary>
	/// ヒットシェーダーのディスクリプタテーブル
	/// </summary>
	enum EHitShaderDescriptorTable {
		eHitShaderDescriptorTable_SRV_CBV,	//SRVとCBV
		eHitShaderDescriptorTable_Sampler,	//サンプラ
		eHitShaderDescriptorTable_Num       //テーブルの数。
	};

	class BLASBuffer {
	public:
		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="commandList"></param>
		/// <param name="instance"></param>
		void Init(ID3D12GraphicsCommandList4* commandList, const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instance);

		/// /// <summary>
		/// BLASBufferのリストを取得。
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
		/// TLASを構築。
		/// </summary>
		/// <param name="rc"></param>
		/// <param name="instances"></param>
		void Init(
			ID3D12GraphicsCommandList4* commandList,
			const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instances,
			const std::vector<AccelerationStructureBuffers>& bottomLevelASBuffers
		);

		/// <summary>
		/// SRVに登録。
		/// </summary>
		/// <param name="descriptorHandle"></param>
		void RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo);// override;

		/// <summary>
		/// VRAM上の仮想アドレスを取得。
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		}

	private:
		AccelerationStructureBuffers m_topLevelASBuffers;
	};

	/// <summary>
	/// レイトレ用のパイプラインステートオブジェクト。
	/// </summary>
	class ReyTracingPSO {
	private:
		/// <summary>
		/// ルートシグネチャ定義。
		/// </summary>
		struct RootSignatureDesc
		{
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			std::vector<D3D12_DESCRIPTOR_RANGE> range;
			std::vector<D3D12_ROOT_PARAMETER> rootParams;
		};

	public:
		/// <summary>
		/// パイプラインステートオブジェクトを初期化。
		/// </summary>
		void Init();

		void QueryInterface(Microsoft::WRL::ComPtr<ID3D12StateObjectProperties>& props) const
		{
			m_pipelineState.As(&props);
			//m_pipelineState->QueryInterface(&props);
		}

		/// <summary>
		/// グローバルルートシグネチャを取得。
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
		RootSignatureDesc CreateRayGenRootSignatureesc();
		RootSignatureDesc CreatePBRMatterialHitRootSignatureDesc();

	private:
		Microsoft::WRL::ComPtr<ID3D12StateObject> m_pipelineState;					//パイプラインステート
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_emptyRootSignature;
	};

	/// <summary>
	/// レイトレワールト゛
	/// </summary>
	class ReyTracingWorld
	{
	public:
		/// <summary>
		/// ジオメトリを登録。
		/// </summary>
		/// <param name="model">モデル</param>
		void RegistGeometry(CModel& model);
		/// <summary>
		/// ジオメトリの登録を確定。
		/// (BLASとTLASを生成)
		/// </summary>
		void CommitRegistGeometry(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// レイトレワールドのインスタンスに対してクエリを行う。
		/// </summary>
		/// <param name="queryFunc"></param>
		void QueryInstances(std::function<void(ReyTracingInstanceData&)> queryFunc) const
		{
			for (auto& instance : m_instances) {
				queryFunc(*instance);
			}
		}

		/// <summary>
		/// インスタンスの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumInstance() const
		{
			return (int)m_instances.size();
		}

	private:
		std::vector<std::unique_ptr<ReyTracingInstanceData>> m_instances;
		BLASBuffer m_blasBuffer;
		TLASBuffer m_topLevelASBuffers;
	};

	/// <summary>
	/// シェーダーテーブル
	/// </summary>
	/// <remark>
	/// シェーダーテーブルはレイのディスパッチで使用される、
	/// シェーダーやリソースのディスクリプタヒープのアドレスなどが登録されているテーブルです。
	/// </remark>
	class ShaderTable {
	public:
		/// <summary>
		/// シェーダーテーブルを初期化。
		/// </summary>
		void Init(
			const ReyTracingWorld& world,
			const ReyTracingPSO& pso
			//const ReyTracingDescriptorHeaps& descriptorHeaps
		);

		/// <summary>
		/// シェーダーテーブルのGPU上の仮想アドレスを取得。
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_shaderTable->GetGPUVirtualAddress();
		}
		/// <summary>
		/// シェーダーテーブルに記憶されているデータの１要素のサイズを取得。
		/// </summary>
		/// <returns></returns>
		uint32_t GetShaderTableEntrySize() const
		{
			return m_shaderTableEntrySize;
		}
		/// <summary>
		/// レイジェネレーションシェーダーの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumRayGenShader() const
		{
			return m_numRayGenShader;
		}
		/// <summary>
		/// ミスシェーダーの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumMissShader() const
		{
			return m_numMissShader;
		}
		/// <summary>
		/// シェーダーテーブルに登録されているヒットシェーダーの数を取得。
		/// </summary>
		/// <remark>
		/// ヒットシェーダーの数はインスタンスの数と同じになります。
		/// </remark>
		/// <returns></returns>
		int GetNumHitShader() const
		{
			return m_numHitShader;
		}
	private:
		/// <summary>
		/// レイジェネレーションシェーダー、ミスシェーダー、ヒットシェーダーの数をカウントする。
		/// </summary>
		void CountupNumGeyGenAndMissAndHitShader();
		/// <summary>
		/// シェーダーテーブルの1要素のサイズを計算する。
		/// </summary>
		void CalcShaderTableEntrySize();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_shaderTable;			//シェーダーテーブル。
		uint32_t m_shaderTableEntrySize = 0;
		int m_numRayGenShader = 0;
		int m_numMissShader = 0;
		int m_numHitShader = 0;
	};

	/// <summary>
	/// レイトレーシングエンジン
	/// </summary>
	class ReyTracingEngine
	{
	public:
		/// <summary>
		/// レイトレーシングをディスパッチ。
		/// </summary>
		/// <param name="rc">レンダリングコンテキスト</param>
		void Dispatch(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// ジオメトリを登録。
		/// </summary>
		/// <param name="model">モデル</param>
		void RegistGeometry(CModel& model)
		{
			//レイトレワールドにジオメトリを登録。
			m_world.RegistGeometry(model);
		}

		/// <summary>
		/// ジオメトリの登録を確定。
		/// </summary>
		void CommitRegistGeometry(ID3D12GraphicsCommandList4* commandList);

	private:
		/// <summary>
		/// シェーダーリソースを作成。
		/// </summary>
		void CreateShaderResources();

	public:
		/// <summary>
		/// 定数バッファ
		/// </summary>
		struct CBStructure {
			CMatrix mRot;	//回転行列
			CVector3 pos;	//視点。
			float aspect;	//アスペクト比。
			float fFar;		//遠平面。
			float fNear;	//近平面。
		};

	private:
		ReyTracingWorld m_world;
		ReyTracingPSO m_pipelineStateObject;
		ShaderTable m_shaderTable;

		ConstantBufferDx12<CBStructure> m_rayGenerationCB;//定数バッファ

		Microsoft::WRL::ComPtr<ID3D12Resource> m_raytracingOutput;//出力バッファ
		D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;//出力バッファのGPUディスクリプタハンドル
	};

}
