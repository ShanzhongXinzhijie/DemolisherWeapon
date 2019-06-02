#pragma once

#include "Graphic/Shader/Shader.h"

namespace DemolisherWeapon {

class SkinModelEffectShader
{
public:
	/// <summary>
	/// シェーダの読み込み
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <param name="entryFuncName">エントリー関数の名前</param>
	/// <param name="shaderType">シェーダの種類</param>
	/// <param name="definesIdentifier">追加の識別子</param>
	/// <param name="pDefines">追加のマクロ</param>
	void Load(const char* filePath, const char* entryFuncName, Shader::EnType shaderType, const char* definesIdentifier = nullptr, const D3D_SHADER_MACRO* pDefines = nullptr);

	/// <summary>
	/// シェーダの取得
	/// </summary>
	/// <param name="macroMask">マクロのマスク</param>
	/// <returns></returns>
	Shader& GetShader(int macroMask) {
		return m_shader[macroMask];
	}

	//シェーダーマクロ
	enum ShaderTypeMask {
		enOFF = 0b0000,			//全て無効
		enMotionBlur = 0b0001,
		enNormalMap = 0b0010,
		enAlbedoMap = 0b0100,
		enLightingMap = 0b1000,
		enALL = 0b1111,			//全て有効
		enNum,					//全パターンの数
	};

private:

	//マクロの数
	static constexpr int MACRO_NUM = 4;
	//シェーダー初期化用配列
	D3D_SHADER_MACRO m_macros[MACRO_NUM + 1] = {
			"MOTIONBLUR", "0",
			"NORMAL_MAP", "0",
			"ALBEDO_MAP", "0",
			"LIGHTING_MAP", "0",
			NULL, NULL
	};

	Shader m_shader[ShaderTypeMask::enNum];
};

//ShaderとSkinModelEffectShaderを同じものとして扱うためのクラス
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
