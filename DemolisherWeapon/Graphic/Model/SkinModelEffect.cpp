#include "DWstdafx.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

ModelEffect::enShaderMode ModelEffect::m_s_shadermode = enNormalShader;

void __cdecl ModelEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	//シェーダーモードにおうじたシェーダをセット
	switch (m_s_shadermode) {
	case enZShader:
		deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSZShader->GetBody(), NULL, 0);
		deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader.GetBody(), NULL, 0);
		break;
	default:
		int macroind = 0;
		if (m_enableMotionBlur) { macroind |= enMotionBlur; }
		if (m_pNormalTex)		{ macroind |= enNormalMap; }

		if (m_pVSShader != &m_vsDefaultShader[enALL]) {
			//カスタムシェーダー
			deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSShader->GetBody(), NULL, 0);
		}
		else {
			//デフォルトシェーダー
			deviceContext->VSSetShader((ID3D11VertexShader*)m_vsDefaultShader[macroind].GetBody(), NULL, 0);
		}
		if (m_pPSShader != &m_psDefaultShader[enALL]) {
			//カスタムシェーダー
			deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSShader->GetBody(), NULL, 0);
		}
		else {
			//デフォルトシェーダー		
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psDefaultShader[macroind].GetBody(), NULL, 0);
		}
		break;
	}

	//テクスチャ
	deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, &m_pAlbedoTex);
	deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, &m_pNormalTex);

	//定数バッファ
	deviceContext->UpdateSubresource(m_materialParamCB, 0, NULL, &m_materialParam, 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB);
}

}