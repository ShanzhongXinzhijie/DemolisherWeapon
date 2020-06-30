/*!
 *@brief	�V�F�[�_�[���\�[�X�B
 *@Flyweight�p�^�[�����g�p�B
 */

#pragma once

namespace DemolisherWeapon {

/*!
 *@brief	�V�F�[�_�[���\�[�X
 *@details
 * �V���O���g���p�^�[���B
 * Flyweight�p�^�[����FlyweightFactory�N���X�B
 */
class ShaderResources{
	/*!
	*@brief	�R���X�g���N�^�B
	*/
	ShaderResources();
	/*!
	*@brief	�f�X�g���N�^�B
	*/
	~ShaderResources();
public:
	
	/*!
	*@brief	�V�F�[�_�[���\�[�X�̃C���X�^���X���擾�B
	*/
	static ShaderResources& GetInstance()
	{
		static ShaderResources instance;
		return instance;
	}

	//�N���X �����N �I�u�W�F�N�g���擾
	ID3D11ClassLinkage* GetClassLinkage() const{
		return m_pClassLinkage;
	}

	/*!
	 * @brief	�V�F�[�_�[�^�C�v�B
	 */
	enum class EnType {
		VS,			//!<���_�V�F�[�_�[�B
		PS,			//!<�s�N�Z���V�F�[�_�[�B
		CS,			//!<�R���s���[�g�V�F�[�_�[�B
	};

	/// <summary>
	/// �V�F�[�_�[���\�[�X�B
	/// �ǂݍ��܂ꂽ�R���p�C���ς݂̃V�F�[�_�[
	/// </summary>
	struct SShaderResource {
		void* shader = nullptr;						//!<�V�F�[�_�[�B
		ID3D11InputLayout* inputLayout = nullptr;	//!<���̓��C�A�E�g�B
		EnType type;								//!<�V�F�[�_�[�^�C�v�B

		//�R���p�C���ς݃V�F�[�_�[�̃f�[�^(�o�C�g�R�[�h)
	private:
		ID3DBlob* blobOut = nullptr;
		std::unique_ptr<char[]> fileblob;
		size_t fileblobSize = 0;
	public:
		LPVOID GetBufferPointer()const {
			if (blobOut) {
				return blobOut->GetBufferPointer();
			}
			if (fileblob) {
				return fileblob.get();
			}
			return nullptr;
		}
		SIZE_T GetBufferSize()const {
			if (blobOut) {
				return blobOut->GetBufferSize();
			}
			if (fileblob) {
				return fileblobSize;
			}
			return 0;
		}

#ifndef DW_MASTER
		//�}�N��
		struct D3D_SHADER_MACRO_SAVE {
			std::unique_ptr<char[]> Name;
			std::unique_ptr<char[]> Definition;
		};
		int macroNum = 0;
		std::unique_ptr<D3D_SHADER_MACRO_SAVE[]> pDefines;
		std::unique_ptr<std::string> entryFuncName;
		int shaderResourceHash = 0;
#endif

		//���I�����N�֌W
		int numInterfaces = 0;
		ID3D11ClassInstance** dynamicLinkageArray = nullptr;

		//�J��
		void Release(bool fullRelease = true);

		friend class ShaderResources;
	};

	/// <summary>
	/// ���[�h
	/// </summary>
	/// <param name="return_resource">(�߂�l)���[�h�����V�F�[�_�[���\�[�X�̃|�C���^</param>
	/// <param name="filePath">���[�h����V�F�[�_�[�̃t�@�C���p�X�B</param>
	/// <param name="entryFuncName">�G���g���[�|�C���g�̊֐����B</param>
	/// <param name="shaderType">�V�F�[�_�[�^�C�v�B</param>
	/// <param name="definesIdentifier">���ʎq�B�}�N���̑g�ݍ��킹���ƂɈႤ���̂��g���Ă�������</param>
	/// <param name="pDefines">�}�N����`</param>
	/// <returns>���[�h������������?</returns>
	bool Load(
		const ShaderResources::SShaderResource*& return_resource,
		std::string_view filePath,
		const char* entryFuncName,
		EnType shaderType,
		const char* definesIdentifier,
		const D3D_SHADER_MACRO* pDefines
	);

	/*!
	*@brief	�J���B
	*@details
	* �����I�ȃ^�C�~���O�ŊJ���������ꍇ�Ɏg�p���Ă��������B
	*/
	void Release();	

#ifndef DW_MASTER
	/// <summary>
	/// �V�F�[�_�[�̃z�b�g�����[�h(�Q�[�����[�v����Ăяo��)
	/// </summary>
	void HotReload();
#endif

	/// <summary>
	/// �t�@�C���p�X���G���W�����̂��̂ɒu�����邩�ݒ�
	/// </summary>
	void SetIsReplaceForEngineFilePath(bool isReplace) {
#ifndef DW_MASTER
		m_replaceForEngineFilePath = isReplace;
#endif
	}

	/// <summary>
	/// �X�V������΃V�F�[�_�[�̍ăR���p�C�����s�����ݒ�
	/// </summary>
	void SetIsRecompile(bool isRecompile) {
		m_isRecompile = isRecompile;
	}

private:
	/// <summary>
	/// �V�F�[�_�[�v���O����
	/// �R���p�C���O�̃V�F�[�_�[�̃f�[�^
	/// </summary>
	struct SShaderProgram {
		std::unique_ptr<char[]> program;
		int programSize;

#ifndef DW_MASTER
		std::string filepath;
		std::filesystem::file_time_type file_time;

		std::list<SShaderResource*> shaderResourceList;//���̃V�F�[�_�[�v���O�����������炽�V�F�[�_�[���\�[�X�̃��X�g
#endif
	};

	typedef std::unique_ptr<SShaderResource> SShaderResourcePtr;
	typedef std::unique_ptr<SShaderProgram>	SShaderProgramPtr;
	std::unordered_map<int, SShaderProgramPtr>	m_shaderProgramMap;		//!<�ǂݍ��ݍς݂̃V�F�[�_�[�v���O�����̃}�b�v�B
	std::unordered_map<int, SShaderResourcePtr>	m_shaderResourceMap;	//!<�V�F�[�_�[���\�[�X�̃}�b�v�B

	ID3D11ClassLinkage* m_pClassLinkage = nullptr;//�N���X �����N �I�u�W�F�N�g

#ifndef DW_MASTER
	bool m_replaceForEngineFilePath = false;//�t�@�C���p�X���G���W�����ɒu�������邩?
#endif

	bool m_isRecompile = true;//�X�V������΃V�F�[�_�[�̍ăR���p�C�����s����

private:
	//�V�F�[�_�v���O���������[�h
	void LoadShaderProgram(const char* filePath, SShaderProgramPtr& return_prog);
	//�V�F�[�_���R���p�C������
	bool CompileShader(const SShaderProgram* shaderProgram, const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, std::string_view hashString, SShaderResource* return_resource, bool isHotReload = false);
	//�V�F�[�_���\�[�X���t�@�C�����烍�[�h
	bool LoadShaderResource(const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, SShaderResource* return_resource);
	//�V�F�[�_���\�[�X�𐶐�������̏���
	bool PostLoadShader(void* blob, size_t blobSize, EnType shaderType, const char* entryFuncName, const D3D_SHADER_MACRO* pDefines, bool isHotReload, SShaderResource* return_resource);
};

}