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
	/// <param name="definesIdentifier">�ǉ��̎��ʎq</param>
	/// <param name="pDefines">�ǉ��̃}�N��</param>
	void Load(const char* filePath, const char* entryFuncName, Shader::EnType shaderType, const char* definesIdentifier = nullptr, const D3D_SHADER_MACRO* pDefines = nullptr);

	/// <summary>
	/// �V�F�[�_�̎擾
	/// </summary>
	/// <param name="macroMask">�}�N���̃}�X�N</param>
	/// <returns></returns>
	const Shader& GetShader(int macroMask)const {
		return m_shader[macroMask];
	}

	//�V�F�[�_�[�}�N��
	enum ShaderTypeMask {
		enOFF = 0b00000,			//�S�Ė���
		enMotionBlur = 0b00001,
		enNormalMap = 0b00010,
		enAlbedoMap = 0b00100,
		enLightingMap = 0b01000,
		enTranslucentMap = 0b10000,
		enALL = 0b11111,			//�S�ėL��
		enNum,						//�S�p�^�[���̐�
	};

private:

	//�}�N���̐�
	static constexpr int MACRO_NUM = 5;
	//�V�F�[�_�[�������p�z��
	D3D_SHADER_MACRO m_macros[MACRO_NUM + 1] = {
			"MOTIONBLUR", "0",
			"NORMAL_MAP", "0",
			"ALBEDO_MAP", "0",
			"LIGHTING_MAP", "0",
			"TRANSLUCENT_MAP", "0",
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

	const Shader* Get(int macroMask)const {
		if (!m_shaderSME) {
			return m_shader;
		}
		else {
			return &m_shaderSME->GetShader(macroMask);
		}
	}

private:
	Shader* m_shader = nullptr;
	SkinModelEffectShader* m_shaderSME = nullptr;
};

}
