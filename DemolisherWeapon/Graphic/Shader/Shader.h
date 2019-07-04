/*!
 * @brief	�V�F�[�_�[�N���X�B
 */

#pragma once

namespace DemolisherWeapon {

/*!
 * @brief	�V�F�[�_�[�B
 * @Flyweight�p�^�[����Flyweight�N���X�B
 */
class Shader {
public:
	/*!
	 * @brief	�V�F�[�_�[�^�C�v�B
	 */
	enum class EnType{
		VS,			//!<���_�V�F�[�_�[�B
		PS,			//!<�s�N�Z���V�F�[�_�[�B
		CS,			//!<�R���s���[�g�V�F�[�_�[�B
	};
	Shader();
	~Shader();
	/*!
	 * @brief	�J���B
	 * @details
	 *	���̊֐��̓f�X�g���N�^���玩���I�ɌĂ΂�܂��B�����I�ȃ^�C�~���O�ŊJ�����s�������ꍇ�Ɏg�p���Ă��������B
	 */
	void Release();
	/*!
	 * @brief	���[�h�B
	 *@param[in]	filePath		�t�@�C���p�X�B
	 *@param[in]	entryFuncName	�G���g���[�|�C���g�ƂȂ�֐��̖��O�B
	 *@param[in]	shaderType		�V�F�[�_�[�^�C�v�B
	 *@return	false���Ԃ��Ă����烍�[�h���s�B
	 */
	bool Load(const char* filePath, const char* entryFuncName, EnType shaderType, const char* definesIdentifier = "", const D3D_SHADER_MACRO* pDefines = nullptr);
	/*!
	* @brief	
	*/
	void* GetBody()
	{
		return m_shader;
	}
	/*!
	* @brief	�C���v�b�g���C�A�E�g���擾�B
	*/
	ID3D11InputLayout* GetInputLayout()
	{
		return m_inputLayout;
	}
	void* GetByteCode()
	{
		return m_blobOut->GetBufferPointer();
	}
	size_t GetByteCodeSize()
	{
		return m_blobOut->GetBufferSize();
	}

	//���I�����N�֌W
	//�C���^�[�t�F�C�X�C���X�^���X�̐����擾
	int GetNumInterfaces()const {
		return m_numInterfaces;
	}
	//�C���^�[�t�F�C�X�C���X�^���X�̔z����擾
	ID3D11ClassInstance** GetClassInstanceArray()const {
		return m_dynamicLinkageArray;
	}

	//���[�h�ς݂��擾
	bool GetIsLoaded()const { return m_isLoaded; }

private:
	bool m_isLoaded = false;//���[�h�ς�?

	void*				m_shader = nullptr;					//!<�V�F�[�_�[�B
	ID3D11InputLayout*	m_inputLayout = nullptr;			//!<���̓��C�A�E�g�B
	ID3DBlob*			m_blobOut = nullptr;

	//���I�����N�֌W
	int m_numInterfaces = 0;
	ID3D11ClassInstance** m_dynamicLinkageArray = nullptr;
};

}