/*!
 * @brief	�V�F�[�_�[�N���X�B
 */

#pragma once

#include"ShaderResources.h"

namespace DemolisherWeapon {

/*!
 * @brief	�V�F�[�_�[�B
 * @Flyweight�p�^�[����Flyweight�N���X�B
 */
class Shader {
public:
	using EnType = ShaderResources::EnType;

	Shader() = default;

	/// <summary>
	/// ���[�h�B
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X�B</param>
	/// <param name="entryFuncName">�G���g���[�|�C���g�ƂȂ�֐��̖��O�B</param>
	/// <param name="shaderType">�V�F�[�_�[�^�C�v�B</param>
	/// <param name="definesIdentifier">���ʎq�B�}�N����`���ƂɈႤ���̂��g���Ă�������</param>
	/// <param name="pDefines">�}�N����`</param>
	/// <returns>false���Ԃ��Ă����烍�[�h���s�B</returns>
	bool Load(const char* filePath, const char* entryFuncName, EnType shaderType, const char* definesIdentifier = "", const D3D_SHADER_MACRO* pDefines = nullptr);

	/*!
	* @brief	
	*/
	void* GetBody()const
	{
		return m_pShaderResource->shader;
	}
	/*!
	* @brief	�C���v�b�g���C�A�E�g���擾�B
	*/
	ID3D11InputLayout* GetInputLayout()const
	{
		return m_pShaderResource->inputLayout;
	}
	void* GetByteCode()const
	{
		return m_pShaderResource->blobOut->GetBufferPointer();
	}
	size_t GetByteCodeSize()const
	{
		return m_pShaderResource->blobOut->GetBufferSize();
	}

	//���I�����N�֌W
	//�C���^�[�t�F�C�X�C���X�^���X�̐����擾
	int GetNumInterfaces()const {
		return m_pShaderResource->numInterfaces;
	}
	//�C���^�[�t�F�C�X�C���X�^���X�̔z����擾
	ID3D11ClassInstance** GetClassInstanceArray()const {
		return m_pShaderResource->dynamicLinkageArray;
	}

	//���[�h�ς݂��擾
	bool GetIsLoaded()const { return m_isLoaded; }

private:
	bool m_isLoaded = false;//���[�h�ς�?
	const ShaderResources::SShaderResource* m_pShaderResource = nullptr;
};

}