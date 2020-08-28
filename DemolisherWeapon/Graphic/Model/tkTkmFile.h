/// <summary>
/// tkm�t�@�C���B
/// </summary>
/// <remarks>
/// tkm�t�@�C����tkEngine�Ǝ���3D���f���t�H�[�}�b�g�ł��B
/// �{�N���X��tkm�t�@�C���̃��[�_�[�ł��B
/// </remarks>

#pragma once

namespace tkEngine {
	
	struct Noncopyable {
		Noncopyable() = default;
		Noncopyable(const Noncopyable&) = delete;
		Noncopyable& operator=(const Noncopyable&) = delete;
	};

	/// <summary>
	/// ���\�[�X�̃C���^�[�t�F�[�X�N���X�B
	/// </summary>
	class IResource : Noncopyable {
	public:
		/// <summary>
		/// �f�X�g���N�^�B
		/// </summary>
		virtual ~IResource();
		/// <summary>
		/// �T�u�N���X�Ŏ�������ǂݍ��ݏ����̖{�́B
		/// </summary>
		virtual void LoadImplement(const char* filePath) = 0;
		/// <summary>
		/// �������[�h�B
		/// </summary>
		/// <param name="filePath"></param>
		void Load(const char* filePath)
		{
			m_filePath = filePath;
			LoadImplement(filePath);
		}
		/// <summary>
		/// �񓯊����[�h�B
		/// </summary>
		/// <param name="filePath"></param>
		void LoadAsync(const char* filePath);
		/// <summary>
		/// �ǂݍ��ݏI������B
		/// </summary>
		/// <returns></returns>
		bool IsLoaded() const
		{
			return m_isLoaded;
		}
	protected:

		/// <summary>
		/// ���[�h�ς݂ɂ���B
		/// </summary>
		void SetLoadedMark()
		{
			m_isLoaded = true;
		}
	private:
		std::string m_filePath;
		bool m_isLoaded = false;	//�ǂݍ��݊����t���O�B
		std::unique_ptr< std::thread > m_loadThread;
	};

	/// <summary>
	/// tkm�t�@�C���N���X�B
	/// </summary>
	class  CTkmFile : public IResource {
	public:
		/// <summary>
		/// �}�e���A��
		/// </summary>
		struct SMaterial {
			std::string albedoMapFileName;			//�A���x�h�}�b�v�̃t�@�C�����B
			std::string normalMapFileName;			//�@���}�b�v�̃t�@�C�����B
			std::string specularMapFileName;		//�X�y�L�����}�b�v�̃t�@�C�����B
			std::unique_ptr<char[]>	albedoMap;		//���[�h���ꂽ�A���x�h�}�b�v�B(dds�t�@�C��)
			unsigned int albedoMapSize;				//�A���x�h�}�b�v�̃T�C�Y�B(dds�t�@�C��)
			std::unique_ptr<char[]>	normalMap;		//���[�h���ꂽ�@���}�b�v�B(dds�t�@�C��)
			unsigned int normalMapSize;				//�@���}�b�v�̃T�C�Y�B
			std::unique_ptr<char[]>	specularMap;	//���[�h���ꂽ�X�y�L�����}�b�v�B(dds�t�@�C��)
			unsigned int specularMapSize;			//�X�y�L�����}�b�v�̃T�C�Y�B(dds�t�@�C��)
		};
		/// <summary>
		/// ���_�B
		/// </summary>
		struct SVertex {
			CVector3 pos;			//���W�B
			CVector3 normal;		//�@���B
			CVector3 tangent;		//�ڃx�N�g���B
			CVector3 binormal;		//�]�x�N�g���B
			CVector2 uv;			//UV���W�B
			int indices[4];			//�X�L���C���f�b�N�X�B
			CVector4 skinWeights;	//�X�L���E�F�C�g�B
		};
		/// <summary>
		/// 32�r�b�g�̃C���f�b�N�X�o�b�t�@�B
		/// </summary>
		struct SIndexBuffer32 {
			std::vector< uint32_t > indices;	//�C���f�b�N�X�B
		};
		/// <summary>
		/// 16�r�b�g�̃C���f�b�N�X�o�b�t�@�B
		/// </summary>
		struct SIndexbuffer16 {
			std::vector< uint16_t > indices;	//�C���f�b�N�X�B
		};
		/// <summary>
		/// ���b�V���p�[�c�B
		/// </summary>
		struct SMesh {
			bool isFlatShading;							//�t���b�g�V�F�[�f�B���O�H
			std::vector< SMaterial > materials;				//�}�e���A���̔z��B
			std::vector< SVertex >	vertexBuffer;			//���_�o�b�t�@�B
			std::vector<SIndexBuffer32> indexBuffer32Array;	//�C���f�b�N�X�o�b�t�@�̔z��B�}�e���A���̐��������C���f�b�N�X�o�b�t�@�͂����B
			std::vector< SIndexbuffer16> indexBuffer16Array;
		};
		
		/// <summary>
		/// 3D���f�������[�h�B
		/// </summary>
		/// <param name="filePath">�t�@�C���p�X�B</param>
		void LoadImplement(const char* filePath) override final;
		
		/// <summary>
		/// ���b�V���p�[�c�ɑ΂��ăN�G�����s���B
		/// </summary>
		/// <param name="func">�N�G���֐�</param>
		void QueryMeshParts(std::function<void(const SMesh& mesh)> func) const
		{
			for (auto& mesh : m_meshParts) {
				func(mesh);
			}
		}
		/// <summary>
		/// ���b�V���̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumMesh() const
		{
			return (int)(m_meshParts.size());
		}
	private:
		/// <summary>
		/// �e�N�X�`���������[�h�B
		/// </summary>
		/// <param name="fp"></param>
		/// <returns></returns>
		std::string LoadTextureFileName(FILE* fp);
		/// <summary>
		/// �C���f�b�N�X�o�b�t�@�����[�h�B
		/// </summary>
		template<class T>
		void LoadIndexBuffer(std::vector<T>& indexBuffer, int numIndex, FILE* fp);
		/// <summary>
		/// �}�e���A�����\�z�B
		/// </summary>
		/// <param name="tkmMat"></param>
		void BuildMaterial(SMaterial& tkmMat, FILE* fp, const char* filePath);
		/// <summary>
		/// �ڃx�N�g���Ə]�x�N�g�����v�Z����B
		/// </summary>
		/// <remarks>
		/// 3dsMaxScript�ł��ׂ��Ȃ񂾂낤���ǁA�f�o�b�O�������̂ō��͂�����ł��B
		/// </remarks>
		void BuildTangentAndBiNormal();
	private:
		std::vector< SMesh>	m_meshParts;		//���b�V���p�[�c�B
	};
}
