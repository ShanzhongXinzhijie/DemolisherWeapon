#include "DWstdafx.h"
#include "SkinModelEffectShader.h"

namespace DemolisherWeapon {

void SkinModelEffectShader::Load(const char* filePath, const char* entryFuncName, Shader::EnType shaderType) {
	//マクロごとにシェーダを作成
	char macroName[32];
	for (int i = 0; i < ShaderTypeMask::enNum; i++) {
		sprintf_s(macroName, "SkinModelEffectShader:%d", i);

		for (int mask = 0; mask < MACRO_NUM; mask++) {
			if (i & BIT(mask)) { macros[mask].Definition = "1"; }else{ macros[mask].Definition = "0"; }
		}

		m_shader[i].Load(filePath, entryFuncName, shaderType, macroName, macros);
	}
}

}