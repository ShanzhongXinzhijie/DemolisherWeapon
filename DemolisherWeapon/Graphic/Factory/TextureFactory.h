#pragma once

namespace DemolisherWeapon {
	/// <summary>
	/// �e�N�X�`��(�r���{�[�h)��FlyweightFactory
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

	public:
		//�e�N�X�`���f�[�^
		struct TextueData {
		private:
			ID3D11Resource* texture = nullptr;
			ID3D11ShaderResourceView* textureView = nullptr;
		public:
			bool isDDS = false;
			UINT width = 0, height = 0;

			friend TextureFactory;
		};

		/// <summary>
		/// �e�N�X�`���̃��[�h
		/// </summary>
		/// <param name="filepath">�t�@�C���p�X</param>
		/// <param name="return_texture">�e�N�X�`�����\�[�X���Ԃ��Ă���</param>
		/// <param name="return_textureView">�e�N�X�`��SRV���Ԃ��Ă���</param>
		/// <param name="return_textureData">�e�N�X�`���̃f�[�^���Ԃ��Ă���</param>
		bool Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture = nullptr, ID3D11ShaderResourceView** return_textureView = nullptr, const TextueData** return_textureData = nullptr);

		/// <summary>
		/// �e�N�X�`���̊J��
		/// </summary>
		void Release();

	private:		
		std::unordered_map<int, TextueData> m_textureMap;
	};
}