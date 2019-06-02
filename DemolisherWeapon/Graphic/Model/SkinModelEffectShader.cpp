#include "DWstdafx.h"
#include "SkinModelEffectShader.h"

namespace DemolisherWeapon {

void SkinModelEffectShader::Load(const char* filePath, const char* entryFuncName, Shader::EnType shaderType, const char* definesIdentifier, const D3D_SHADER_MACRO* pDefines) {
	
	//追加のマクロ
	std::unique_ptr<D3D_SHADER_MACRO[]> combinedMacro;
	std::unique_ptr<char[]> combinedIdentifier;
	int strCnt = 0;
	int macroCnt = 0;
	if (definesIdentifier && pDefines) {
		//追加のマクロの数カウント
		int i = 0;
		while (1) {
			if (pDefines[i].Name && pDefines[i].Definition) {
				macroCnt++;
			}
			else {
				break;
			}
			i++;
		}
		//追加の識別子の文字数カウント
		strCnt = strlen(definesIdentifier);
		
		//確保
		combinedMacro = std::make_unique<D3D_SHADER_MACRO[]>(MACRO_NUM + macroCnt + 1);
		combinedIdentifier = std::make_unique<char[]>(32 + strCnt);
		//コピー
		for (int i = 0; i < macroCnt; i++) {
			combinedMacro[i] = pDefines[i];
		}
		combinedMacro[MACRO_NUM + macroCnt].Name = nullptr;
		combinedMacro[MACRO_NUM + macroCnt].Definition = nullptr;
	}

	//マクロごとにシェーダを作成
	char macroName[32];
	for (int i = 0; i < ShaderTypeMask::enNum; i++) {
		sprintf_s(macroName, "SkinModelEffectShader:%d", i);

		for (int mask = 0; mask < MACRO_NUM; mask++) {
			if (i & BIT(mask)) { m_macros[mask].Definition = "1"; }else{ m_macros[mask].Definition = "0"; }
			//追加のマクロ
			if (definesIdentifier && pDefines) {
				combinedMacro[macroCnt + mask].Name = m_macros[mask].Name;
				combinedMacro[macroCnt + mask].Definition = m_macros[mask].Definition;
			}
		}

		if (definesIdentifier && pDefines) {
			//追加のマクロ
			sprintf_s(combinedIdentifier.get(), 32 + strCnt, "%s%s", macroName, definesIdentifier);
			m_shader[i].Load(filePath, entryFuncName, shaderType, combinedIdentifier.get(), combinedMacro.get());
		}
		else {
			m_shader[i].Load(filePath, entryFuncName, shaderType, macroName, m_macros);
		}
	}
}

}