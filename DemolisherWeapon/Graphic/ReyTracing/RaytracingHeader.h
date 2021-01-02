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
	/// レイトレ用ジオメトリ情報
	/// </summary>
	struct ReyTracingGeometoryData {
		D3D12_RAYTRACING_GEOMETRY_DESC m_geometoryDesc;	//ジオメトリ情報
		StructuredBufferInnerDX12 m_vertexBufferRWSB;	//頂点バッファ
		StructuredBufferInnerDX12 m_indexBufferRWSB;	//インデックスバッファ

		//AccelerationStructureBuffers m_BLAS;
		ID3D12Resource* m_pResultBLAS = nullptr;
	};
	/// <summary>
	/// レイトレ用インスタンス情報
	/// </summary>
	struct ReyTracingInstanceData {
		const ReyTracingGeometoryData* m_geometory = nullptr;	//ジオメトリ情報
		const MaterialSetting* m_material = nullptr;			//マテリアル
		const CMatrix* m_worldMatrix = nullptr;					//ワールド行列
	};

	//レイトレースの再帰呼び出しの最大数。
	//これがTraceRayを再帰的に呼び出せる最大数です。
	inline constexpr int MAX_TRACE_RECURSION_DEPTH = 4;

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
		eShader_ShadowAny,
		eShader_ShadowMiss,			//シャドウレイがどこにもぶつからなかった時に呼ばれるシェーダー。
		eShader_Num,				//シェーダーの数。
	};
	//シェーダーのカテゴリ。
	enum EShaderCategory {
		eShaderCategory_RayGenerator,	//レイを生成するシェーダー。
		eShaderCategory_Miss,			//ミスシェーダー。
		eShaderCategory_ClosestHit,		//もっとも近いポリゴンとレイが交差したときに呼ばれるシェーダー。
		eShaderCategory_AnyHit,			//Any hit shader
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
		{ L"shadowAny",		eLocalRootSignature_PBRMaterialHit,	eShaderCategory_AnyHit,			eHitGroup_PBRShadowRay },
		{ L"shadowMiss",	eLocalRootSignature_Empty,			eShaderCategory_Miss,			eHitGroup_Undef },
	};
	static_assert(ARRAYSIZE(shaderDatas) == eShader_Num, "shaderDatas arraySize is invalid!! shaderDatas arraySize must be equal to eShader_Num");

	//ヒットグループ構造体。
	struct SHitGroup {
		const wchar_t* name;				//ヒットグループの名前。
		const wchar_t* chsHitShaderName;	//最も近いポリゴンにヒットしたときに呼ばれるシェーダーの名前。
		const wchar_t* anyHitShaderName;	//any hit shader
	};
	constexpr inline  SHitGroup hitGroups[] = {
		{ L"HitGroup",			shaderDatas[eShader_PBRChs].entryPointName,	shaderDatas[eShader_ShadowAny].entryPointName },
		{ L"ShadowHitGroup",	shaderDatas[eShader_ShadowChs].entryPointName, shaderDatas[eShader_ShadowAny].entryPointName },
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
		eWorldMatrixs,						//ワールド行列
		eNum,								//SRVの数。
		eNumRayGenerationSRV = eEndRayGenerationSRV - eStartRayGenerationSRV,//レイジェネレーションシェーダーで使用するSRVの数。
	};

	enum EHitShaderDescriptorTable {
		eHitShaderDescriptorTable_SRV_CBV,	//SRVとCBV
		eHitShaderDescriptorTable_Sampler,	//サンプラ
		eHitShaderDescriptorTable_Num       //テーブルの数。
	};

	/// <summary>
	/// 定数バッファ
	/// </summary>
	struct ReyTracingCBStructure {
		CMatrix mRot;	//回転行列
		CVector3 pos;	//視点。
		float fFar;		//遠平面。
		float fNear;	//近平面。
		float fov;      //視野角
	};
}