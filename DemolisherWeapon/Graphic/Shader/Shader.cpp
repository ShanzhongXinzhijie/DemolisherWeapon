/*!
 * @brief	シェーダー。
 */
#include "DWstdafx.h"
#include "Shader.h"

namespace DemolisherWeapon {
	
bool Shader::Load(const char* filePath, const char* entryFuncName, EnType shaderType, const char* definesIdentifier, const D3D_SHADER_MACRO* pDefines)
{
	bool result = ShaderResources::GetInstance().Load(
		m_pShaderResource,
		filePath,
		entryFuncName,
		shaderType,
		definesIdentifier,
		pDefines
	);
	if (!result) {
		MessageBox(NULL, filePath, "Shader::Load()に失敗しました", MB_OK);
	}
	m_isLoaded = result;
    return result;
}

}