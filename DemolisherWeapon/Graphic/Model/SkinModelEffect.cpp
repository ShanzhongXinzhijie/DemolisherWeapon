#include "DWstdafx.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

#ifndef DW_DX12_TEMPORARY

void __cdecl ModelEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	//�V�F�[�_�[���[�h�ɂ��������V�F�[�_���Z�b�g
	switch (GetGraphicsEngine().GetModelDrawMode().GetShaderMode()) {
	case ModelDrawMode::enZShader:
		//Z�l�̕`��
		
		//���_�V�F�[�_
		deviceContext->VSSetShader((ID3D11VertexShader*)m_matData.m_ptrUseMaterialSetting->GetVSZ()->GetBody(), NULL, 0);
	
		//�s�N�Z���V�F�[�_
		if (m_matData.m_ptrUseMaterialSetting->GetPSZ() == &m_matData.m_psZShader[0]) {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_matData.m_psZShader[m_matData.m_ptrUseMaterialSetting->GetIsUseTexZShader() ? 1 : 0].GetBody(), NULL, 0);
		}
		else {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_matData.m_ptrUseMaterialSetting->GetPSZ()->GetBody(), NULL, 0);
		}

		break;
	default:
		//�ʏ�`��

		//�f�t�H���g�V�F�[�_�̃}�N����؂�ւ���
		int macroind = 0;
		if (m_matData.m_ptrUseMaterialSetting->GetIsMotionBlur())		{ macroind |= SkinModelEffectShader::enMotionBlur; }
		if (m_matData.m_ptrUseMaterialSetting->GetNormalTexture())		{ macroind |= SkinModelEffectShader::enNormalMap; }
		if (m_matData.m_ptrUseMaterialSetting->GetAlbedoTexture())		{ macroind |= SkinModelEffectShader::enAlbedoMap; }
		if (m_matData.m_ptrUseMaterialSetting->GetLightingTexture())	{ macroind |= SkinModelEffectShader::enLightingMap; }
		if (m_matData.m_ptrUseMaterialSetting->GetTranslucentTexture())	{ macroind |= SkinModelEffectShader::enTranslucentMap; }

		//���_�V�F�[�_
		deviceContext->VSSetShader((ID3D11VertexShader*)m_matData.m_ptrUseMaterialSetting->GetVS().Get(macroind)->GetBody(), NULL, 0);

		//�s�N�Z���V�F�[�_
		deviceContext->PSSetShader((ID3D11PixelShader*)m_matData.m_ptrUseMaterialSetting->GetPS().Get(macroind)->GetBody(), NULL, 0);
		
		break;
	}

	//�e�N�X�`��
	if (m_matData.m_ptrUseMaterialSetting->GetAlbedoTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, m_matData.m_ptrUseMaterialSetting->GetAddressOfAlbedoTexture());
	}
	if (m_matData.m_ptrUseMaterialSetting->GetNormalTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, m_matData.m_ptrUseMaterialSetting->GetAddressOfNormalTexture());
	}
	if (m_matData.m_ptrUseMaterialSetting->GetLightingTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_LightngTexture, 1, m_matData.m_ptrUseMaterialSetting->GetAddressOfLightingTexture());
	}
	if (m_matData.m_ptrUseMaterialSetting->GetTranslucentTexture()) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_TranslucentTexture, 1, m_matData.m_ptrUseMaterialSetting->GetAddressOfTranslucentTexture());
	}

	//�萔�o�b�t�@
	deviceContext->UpdateSubresource(m_matData.m_materialParamCBDX11.Get(), 0, NULL, &m_matData.m_ptrUseMaterialSetting->GetMaterialParam(), 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, m_matData.m_materialParamCBDX11.GetAddressOf());
}

#endif
}