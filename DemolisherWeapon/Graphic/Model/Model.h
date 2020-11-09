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
		static constexpr inline D3D12_INPUT_ELEMENT_DESC InputElementsDX12[InputElementCount] =
		{
			{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
			{ "BLENDINDICES",0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//DXGI_FORMAT_R8G8B8A8_UINT
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//DXGI_FORMAT_R8G8B8A8_UNORM
			//{ "BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		static constexpr inline D3D12_INPUT_ELEMENT_DESC InputElementsDX12[InputElementCount] =
		{
			{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES",0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
	};

	/// <summary>
	/// �}�e���A��
	/// </summary>
	class IMaterial {
	public:
		IMaterial(bool isSkinModel, const tkEngine::CTkmFile::SMaterial& tkmMat, int number = -1);
		virtual ~IMaterial(){}

		/// <summary>
		/// �}�e���A����K�p����
		/// </summary>
		virtual void Apply() = 0;

		/// <summary>
		/// �}�e���A���f�[�^���擾
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
	class PipelineStateDX12 : public IPipelineState {
	public:
		void Init(MaterialData& mat)override;
		void Apply()override;
	private:
	};

	/// <summary>
	/// ���b�V��
	/// </summary>
	struct SModelMesh {
		~SModelMesh() {
			if (m_vertexData) {
				delete[] m_vertexData;
			}
			if (m_vertexDataDXR) {
				delete[] m_vertexDataDXR;
			}
		}

		int m_vertexNum = 0;
		VertexPositionNormalTangentColorTexture* m_vertexData = nullptr;	//���_�f�[�^
		VertexPositionNormalTangentColorTexture* m_vertexDataDXR = nullptr;	//���_�f�[�^(���C�g���p)
		std::vector<std::vector<unsigned long>> m_indexDataArray;			//�C���f�b�N�X�f�[�^

		std::unique_ptr<IVertexBuffer>					m_vertexBuffer;		//���_�o�b�t�@�B
		std::unique_ptr<IVertexBuffer>					m_vertexBufferDXR;		//���_�o�b�t�@�B(���C�g���p)
		std::vector<std::unique_ptr<IIndexBuffer>>		m_indexBufferArray;	//�C���f�b�N�X�o�b�t�@�B
		std::vector<std::unique_ptr<IMaterial>>			m_materials;		//�}�e���A���B
		std::vector<int>								m_skinFlags;		//�X�L���������Ă��邩�ǂ����̃t���O�B
		std::vector<std::unique_ptr<IPipelineState>>	m_pipelineState;	//�p�C�v���C���X�e�[�g
	};

	/// <summary>
	/// ���b�V���p�[�c
	/// </summary>
	class CModelMeshParts {
	public:
		/// <summary>
		/// tkm�t�@�C�����珉�����B
		/// </summary>
		/// <param name="tkmFile"></param>
		void InitFromTkmFile(const tkEngine::CTkmFile& tkmFile);

		/// <summary>
		/// �`��B
		/// </summary>
		void Draw(int instanceNum);

		std::vector< std::unique_ptr<SModelMesh> > m_meshs;//���b�V��

	private:
		void CreateMeshFromTkmMesh(const tkEngine::CTkmFile::SMesh& tkmMesh, int meshNo, bool isRayTrace = true);

	};

	/// <summary>
	/// ���f���N���X�B
	/// </summary>
	class CModel {
	public:
		//���b�V���E�}�e���A���̌���

		/// <summary>
		/// tkm�t�@�C����񓯊����\�h�B
		/// </summary>
		/// <remarks>
		/// IsInited�֐��𗘗p���āA�������Ƃ�悤�ɂ��Ă��������B
		/// </remarks>
		/// <param name="filePath">tkm�t�@�C���̃t�@�C���p�X�B</param>
		void LoadTkmFileAsync(const char* filePath);

		/// <summary>
		/// tkm�t�@�C�������[�h�B
		/// </summary>
		/// <param name="filePath">tkm�t�@�C���̃t�@�C���p�X�B</param>
		void LoadTkmFile(const char* filePath);

		/// <summary>
		/// ���b�V���p�[�c���쐬����B
		/// </summary>
		/// <remarks>
		/// ���̊֐���LoadTkmFileAsync�֐��Ȃǂ𗘗p���āA
		/// tkm�t�@�C�������[�h������ŌĂяo���Ă��������B
		/// </remarks>
		void CreateMeshParts();

		/// <summary>
		/// ���������I����Ă��邩����B
		/// </summary>
		/// <returns></returns>
		bool IsInited() const;

		/// <summary>
		/// �`��
		/// </summary>
		void Draw(int instanceNum);

		/// <summary>
		/// ���b�V���̌���
		/// </summary>
		/// <param name="onFindMesh">�����������b�V���������Ɏ��֐�</param>
		void FindMesh(std::function<void(const std::unique_ptr<SModelMesh>&)> onFindMesh) {
			for (auto& mesh : m_meshParts.m_meshs) {
				onFindMesh(mesh);
			}
		}

		/// <summary>
		/// tkm�t�@�C�����擾�B
		/// </summary>
		/// <returns></returns>
		const tkEngine::CTkmFile& GetTkmFile() const
		{
			return m_tkmFile;
		}

	private:
		tkEngine::CTkmFile m_tkmFile;	//tkm�t�@�C���B
		CModelMeshParts m_meshParts;			//���b�V���p�[�c�B
	};
}