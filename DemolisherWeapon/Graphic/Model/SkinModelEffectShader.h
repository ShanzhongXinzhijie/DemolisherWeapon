#pragma once

#include "Graphic/Shader/Shader.h"

namespace DemolisherWeapon {

class SkinModelEffectShader
{
public:
	/// <summary>
	/// �V�F�[�_�̓ǂݍ���
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	/// <param name="entryFuncName">�G���g���[�֐��̖��O</param>
	/// <param name="shaderType">�V�F�[�_�̎��</param>
	void Load(const char* filePath, const char* entryFuncName, Shader::EnType shaderType);

	/// <summary>
	/// �V�F�[�_�̎擾
	/// </summary>
	/// <param name="macroMask">�}�N���̃}�X�N</param>
	/// <returns></returns>
	Shader& GetShader(int macroMask) {
		return m_shader[macroMask];
	}

	//�V�F�[�_�[�}�N��
	enum ShaderTypeMask {
		enOFF = 0b0000,			//�S�Ė���
		enMotionBlur = 0b0001,
		enNormalMap = 0b0010,
		enAlbedoMap = 0b0100,
		enLightingMap = 0b1000,
		enALL = 0b1111,			//�S�ėL��
		enNum,					//�S�p�^�[���̐�
	};

private:

	//�}�N���̐�
	static constexpr int MACRO_NUM = 4;
	//�V�F�[�_�[�������p�z��
	D3D_SHADER_MACRO macros[MACRO_NUM + 1] = {
			"MOTIONBLUR", "0",
			"NORMAL_MAP", "0",
			"ALBEDO_MAP", "0",
			"LIGHTING_MAP", "0",
			NULL, NULL
	};

	Shader m_shader[ShaderTypeMask::enNum];
};

//Shader��SkinModelEffectShader�𓯂����̂Ƃ��Ĉ������߂̃N���X
class SKEShaderPtr {
public:
	SKEShaderPtr() {}
	SKEShaderPtr(Shader* s) : m_shader(s) {}
	SKEShaderPtr(SkinModelEffectShader* s) : m_shaderSME(s) {}

	Shader* Get(int macroMask) {
		if (!m_shaderSME) {
			return m_shader;
		}
		else {
			return &m_shaderSME->GetShader(macroMask) ;
		}
	}
private:
	Shader* m_shader = nullptr;
	SkinModelEffectShader* m_shaderSME = nullptr;
};

}
