#include "DWstdafx.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

ModelEffect::enShaderMode ModelEffect::m_s_shadermode = enNormalShader;

#ifndef DW_DX12_TEMPORARY

void __cdecl ModelEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	//シェーダーモードにおうじたシェーダをセット
	switch (m_s_shadermode) {
	case enZShader: 
		//Z値の描画
		
		//頂点シェーダ
		deviceContext->VSSetShader((ID3D11VertexShader*)m_ptrUseMaterialSetting->GetVSZ()->GetBody(), NULL, 0);
	
		//ピクセルシェーダ
		if (m_ptrUseMaterialSetting->GetPSZ() == &m_psZShader[0]) {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader[m_ptrUseMaterialSetting->GetIsUseTexZShader() ? 1 : 0].GetBody(), NULL, 0);
		}
		else {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_ptrUseMaterialSetting->GetPSZ()->GetBody(), NULL, 0);
		}

		break;
	default:
		//通常描画

		//デフォルトシェーダのマクロを切り替える
		int macroind = 0;
		if (m_ptrUseMaterialSetting->GetIsMotionBlur())			{ macroind |= SkinModelEffectShader::enMotionBlur; }
		if (m_ptrUseMaterialSetting->GetNormalTexture())		{ macroind |= SkinModelEffectShader::enNormalMap; }
		if (m_ptrUseMaterialSetting->GetAlbedoTexture())		{ macroind |= SkinModelEffectShader::enAlbedoMap; }
		if (m_ptrUseMaterialSetting->GetLightingTexture())		{ macroind |= SkinModelEffectShader::enLightingMap; }
		if (m_ptrUseMaterialSetting->GetTranslucentTexture())	{ macroind |= SkinModelEffectShader::enTranslucentMap; }

		//頂点シェーダ
		deviceContext->VSSetShader((ID3D11VertexShader*)m_ptrUseMaterialSetting->GetVS().Get(macroind)->GetBody(), NULL, 0);

		//ピクセルシェーダ
		deviceContext->PSSetShader((ID3D11PixelShader*)m_ptrUseMaterialSetting->GetPS().Get(macroind)->GetBody(), NULL, 0);
		
		break;
	}

	//テクスチャ
	if (m_ptrUseMaterialSetting->GetAlbedoTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, m_ptrUseMaterialSetting->GetAddressOfAlbedoTexture());
	}
	if (m_ptrUseMaterialSetting->GetNormalTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, m_ptrUseMaterialSetting->GetAddressOfNormalTexture());
	}
	if (m_ptrUseMaterialSetting->GetLightingTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_LightngTexture, 1, m_ptrUseMaterialSetting->GetAddressOfLightingTexture());
	}
	if (m_ptrUseMaterialSetting->GetTranslucentTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_TranslucentTexture, 1, m_ptrUseMaterialSetting->GetAddressOfTranslucentTexture());
	}

	//定数バッファ
	deviceContext->UpdateSubresource(m_materialParamCB, 0, NULL, &m_ptrUseMaterialSetting->GetMaterialParam(), 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB);
}

#endif
}