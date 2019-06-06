#include "DWstdafx.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

ModelEffect::enShaderMode ModelEffect::m_s_shadermode = enNormalShader;

void __cdecl ModelEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	//�V�F�[�_�[���[�h�ɂ��������V�F�[�_���Z�b�g
	switch (m_s_shadermode) {
	case enZShader: 
		//Z�l�̕`��
		deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSZShader->GetBody(), NULL, 0);
		if (m_pPSZShader == &m_psZShader[0]) {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader[m_isUseTexZShader ? 1 : 0].GetBody(), NULL, 0);
		}
		else {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSZShader->GetBody(), NULL, 0);
		}
		break;
	default:
		//�ʏ�`��

		//�f�t�H���g�V�F�[�_�̃}�N����؂�ւ���
		int macroind = 0;
		if (m_enableMotionBlur) { macroind |= SkinModelEffectShader::enMotionBlur; }
		if (m_pNormalTex)		{ macroind |= SkinModelEffectShader::enNormalMap; }
		if (m_pAlbedoTex)		{ macroind |= SkinModelEffectShader::enAlbedoMap; }
		if (m_pLightingTex)		{ macroind |= SkinModelEffectShader::enLightingMap; }

		//���_�V�F�[�_
		deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSShader.Get(macroind)->GetBody(), NULL, 0);

		//�s�N�Z���V�F�[�_
		deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSShader.Get(macroind)->GetBody(), NULL, 0);
		
		break;
	}

	//�e�N�X�`��
	if (m_pAlbedoTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, &m_pAlbedoTex);
	}
	if (m_pNormalTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, &m_pNormalTex);
	}
	if (m_pLightingTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_LightngTexture, 1, &m_pLightingTex);
	}

	//�萔�o�b�t�@
	deviceContext->UpdateSubresource(m_materialParamCB, 0, NULL, &m_materialParam, 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB);
}

}