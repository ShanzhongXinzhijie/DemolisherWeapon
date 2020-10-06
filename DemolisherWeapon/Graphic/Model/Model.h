#pragma once
#include"tktkmfile.h"
#include"Graphic/Model/MaterialSetting.h"
#include"Graphic/CPrimitive.h"

namespace DemolisherWeapon{

	struct VertexPositionNormalTangentColorTexture
	{
		VertexPositionNormalTangentColorTexture() = default;

		CVector3 position;
		CVector3 normal;
		CVector4 tangent;
		//uint32_t color;
		CVector2 textureCoordinate;

		//void __cdecl SetColor(XMFLOAT4 const& icolor) { SetColor(XMLoadFloat4(&icolor)); }
		//void XM_CALLCONV SetColor(FXMVECTOR icolor);

		static constexpr int InputElementCount = 4;
		static constexpr D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount] =
		{
			{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "COLOR",       0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	};


	// Vertex struct for Visual Studio Shader Designer (DGSL) holding position, normal,
	// tangent, color (RGBA), texture mapping information, and skinning weights
	struct VertexPositionNormalTangentColorTextureSkinning : public VertexPositionNormalTangentColorTexture
	{
		VertexPositionNormalTangentColorTextureSkinning() = default;

		//uint32_t indices;
		//uint32_t weights;
		unsigned int indices[4];			
		CVector4 weights;

		//void __cdecl SetBlendIndices(DirectX::XMUINT4 const& iindices);

		//void __cdecl SetBlendWeights(DirectX::XMFLOAT4 const& iweights) { SetBlendWeights(XMLoadFloat4(&iweights)); }
		//void XM_CALLCONV SetBlendWeights(DirectX::FXMVECTOR iweights);

		static constexpr int InputElementCount = 6;
		static constexpr D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount] =
		{
			{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "COLOR",       0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES",0, DXGI_FORMAT_R32G32B32A32_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//DXGI_FORMAT_R8G8B8A8_UINT
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//DXGI_FORMAT_R8G8B8A8_UNORM
			//{ "BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	};

	/// <summary>
	/// マテリアル
	/// </summary>
	class IMaterial {
	public:
		IMaterial(bool isSkinModel, const tkEngine::CTkmFile::SMaterial& tkmMat, int number = -1);
		virtual ~IMaterial(){}

		/// <summary>
		/// マテリアルを適用する
		/// </summary>
		virtual void Apply() = 0;

		/// <summary>
		/// マテリアルデータを取得
		/// </summary>
		MaterialData& GetMaterialData() {
			return m_materialData;
		}

	protected:
		MaterialData m_materialData;
	};
	class MaterialDX11 : public IMaterial {
	public:
		using IMaterial::IMaterial;
		void Apply()override;
	};
	class MaterialDX12 : public IMaterial {
	public:
		using IMaterial::IMaterial;
		void Apply()override;
	};

	/// <summary>
	/// PipelineState
	/// </summary>
	class IPipelineState {
	public:
		virtual ~IPipelineState() {}

		virtual void Init(MaterialData& mat) = 0;
		virtual void Apply() = 0;
	};
	class PipelineStateDX11 : public IPipelineState {
	public:
		void Init(MaterialData& mat)override;
		void Apply()override;
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	};

	/// <summary>
	/// メッシュ
	/// </summary>
	struct SModelMesh {
		~SModelMesh() {
			if (m_vertexData) {
				delete[] m_vertexData;
			}
		}

		int m_vertexNum = 0;
		VertexPositionNormalTangentColorTexture* m_vertexData = nullptr;	//頂点データ
		std::vector<std::vector<unsigned long>> m_indexDataArray;			//インデックスデータ

		std::unique_ptr<IVertexBuffer>					m_vertexBuffer;		//頂点バッファ。
		std::vector<std::unique_ptr<IIndexBuffer>>		m_indexBufferArray;	//インデックスバッファ。
		std::vector<std::unique_ptr<IMaterial>>			m_materials;		//マテリアル。
		std::vector<int>								m_skinFlags;		//スキンを持っているかどうかのフラグ。
		std::vector<std::unique_ptr<IPipelineState>>	m_pipelineState;	//パイプラインステート
	};

	/// <summary>
	/// メッシュパーツ
	/// </summary>
	class CModelMeshParts {
	public:
		/// <summary>
		/// tkmファイルから初期化。
		/// </summary>
		/// <param name="tkmFile"></param>
		void InitFromTkmFile(const tkEngine::CTkmFile& tkmFile);

		/// <summary>
		/// 描画。
		/// </summary>
		void Draw(int instanceNum);

		std::vector< std::unique_ptr<SModelMesh> > m_meshs;//メッシュ

	private:
		void CreateMeshFromTkmMesh(const tkEngine::CTkmFile::SMesh& tkmMesh, int meshNo);

	};

	/// <summary>
	/// モデルクラス。
	/// </summary>
	class CModel {
	public:
		//メッシュ・マテリアルの検索

		/// <summary>
		/// tkmファイルを非同期ロ―ド。
		/// </summary>
		/// <remarks>
		/// IsInited関数を利用して、同期をとるようにしてください。
		/// </remarks>
		/// <param name="filePath">tkmファイルのファイルパス。</param>
		void LoadTkmFileAsync(const char* filePath);

		/// <summary>
		/// tkmファイルをロード。
		/// </summary>
		/// <param name="filePath">tkmファイルのファイルパス。</param>
		void LoadTkmFile(const char* filePath);

		/// <summary>
		/// メッシュパーツを作成する。
		/// </summary>
		/// <remarks>
		/// この関数はLoadTkmFileAsync関数などを利用して、
		/// tkmファイルをロードした後で呼び出してください。
		/// </remarks>
		void CreateMeshParts();

		/// <summary>
		/// 初期化が終わっているか判定。
		/// </summary>
		/// <returns></returns>
		bool IsInited() const;

		/// <summary>
		/// 描画
		/// </summary>
		void Draw(int instanceNum);

		/// <summary>
		/// メッシュの検索
		/// </summary>
		/// <param name="onFindMesh">発見したメッシュを引数に取る関数</param>
		void FindMesh(std::function<void(const std::unique_ptr<SModelMesh>&)> onFindMesh) {
			for (auto& mesh : m_meshParts.m_meshs) {
				onFindMesh(mesh);
			}
		}

		/// <summary>
		/// tkmファイルを取得。
		/// </summary>
		/// <returns></returns>
		const tkEngine::CTkmFile& GetTkmFile() const
		{
			return m_tkmFile;
		}

	private:
		tkEngine::CTkmFile m_tkmFile;	//tkmファイル。
		CModelMeshParts m_meshParts;			//メッシュパーツ。
	};
}