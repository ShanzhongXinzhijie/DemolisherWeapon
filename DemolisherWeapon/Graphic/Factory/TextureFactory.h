#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// �e�N�X�`���f�[�^
	/// </summary>
	struct TextueData {
	public:
		//DirectX12
		Microsoft::WRL::ComPtr<ID3D12Resource> d3d12texture;
		D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandle;
		//DirectX11
		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

		//�v���p�e�B
		bool isDDS = false;//�t�@�C���\�[�X��.dds���ǂ���
		UINT width = 0, height = 0;//�𑜓x

		//���[�h�ς�?
		bool isLoaded()const {
			return (width > 0 && height > 0) ? true : false;
		}
	};

	/// <summary>
	/// �e�N�X�`�����쐬����
	/// </summary>
	inline TextueData CreateTexture(std::experimental::filesystem::path filepath, bool generateMipmaps = false);

	/// <summary>
	/// �e�N�X�`����FlyweightFactory
	/// </summary>
	class TextureFactory {
	//�V���O���g��
	private:
		TextureFactory() = default;
		~TextureFactory() { Release(); }
	public:
		TextureFactory(const TextureFactory&) = delete;
		TextureFactory& operator=(const TextureFactory&) = delete;
		TextureFactory(TextureFactory&&) = delete;
		TextureFactory& operator=(TextureFactory&&) = delete;

	private:
		static TextureFactory* instance;
	public:
		//�C���X�^���X���擾
		static TextureFactory& GetInstance()
		{
			if (instance == nullptr) {
				instance = new TextureFactory;
			}
			return *instance;
		}
		//�C���X�^���X�̍폜
		static void DeleteInstance() {
			if (instance) {
				delete instance; instance = nullptr;
			}
		}
	//

	public:
		/// <summary>
		/// �e�N�X�`���̃��[�h
		/// </summary>
		/// <param name="filepath">�t�@�C���p�X</param>
		/// <param name="return_texture">�e�N�X�`�����\�[�X���Ԃ��Ă���</param>
		/// <param name="return_textureView">�e�N�X�`��SRV���Ԃ��Ă���</param>
		/// <param name="return_textureData">�e�N�X�`���̃f�[�^���Ԃ��Ă���</param>
		/// <param name="generateMipmaps">�~�b�v�}�b�v�𐶐����邩?</param>
		bool Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture = nullptr, ID3D11ShaderResourceView** return_textureView = nullptr, const TextueData** return_textureData = nullptr, bool generateMipmaps = false);

		bool Load(std::experimental::filesystem::path filepath, const TextueData** return_textureData = nullptr, bool generateMipmaps = false);

		/// <summary>
		/// �e�N�X�`���̊J��
		/// </summary>
		void Release();

	private:		
		std::unordered_map<int, TextueData> m_textureMap;
	};
}