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
		//Z値の描画
		deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSZShader->GetBody(), NULL, 0);
		deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader[m_isUseTexZShader ? 1 : 0].GetBody(), NULL, 0);
		break;
	default:
		//通常描画

		//デフォルトシェーダのマクロを切り替える
		int macroind = 0;
		if (m_enableMotionBlur) { macroind |= enMotionBlur; }
		if (m_pNormalTex)		{ macroind |= enNormalMap; }
		if (m_pAlbedoTex)		{ macroind |= enAlbedoMap; }
		if (m_pLightingTex)		{ macroind |= enLightingMap; }

		//頂点シェーダ
		if (m_pVSShader != &m_vsDefaultShader[enALL]) {
			//カスタムシェーダー
			deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSShader->GetBody(), NULL, 0);
		}
		else {
			//デフォルトシェーダー
			deviceContext->VSSetShader((ID3D11VertexShader*)m_vsDefaultShader[macroind].GetBody(), NULL, 0);
		}

		//ピクセルシェーダ
		if (m_pPSShader == &m_psDefaultShader[enALL]) {
			//デフォルトシェーダー		
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psDefaultShader[macroind].GetBody(), NULL, 0);
		}
		else if(m_pPSShader == &m_psTriPlanarMapShader[enALL]) {
			//TriPlanarMapping用のシェーダー
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psTriPlanarMapShader[macroind].GetBody(), NULL, 0);
		}
		else {
			//カスタムシェーダー
			deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSShader->GetBody(), NULL, 0);
		}
		break;
	}

	//テクスチャ
	if (m_pAlbedoTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, &m_pAlbedoTex);
	}
	if (m_pNormalTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, &m_pNormalTex);
	}
	if (m_pLightingTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_LightngTexture, 1, &m_pLightingTex);
	}

	//定数バッファ
	deviceContext->UpdateSubresource(m_materialParamCB, 0, NULL, &m_materialParam, 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB);
}

}