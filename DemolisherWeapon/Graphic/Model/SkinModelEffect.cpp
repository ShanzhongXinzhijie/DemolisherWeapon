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
		deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSZShader->GetBody(), NULL, 0);
		deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader[m_isUseTexZShader ? 1 : 0].GetBody(), NULL, 0);
		break;
	default:
		int macroind = 0;
		if (m_enableMotionBlur) { macroind |= enMotionBlur; }
		if (m_pNormalTex)		{ macroind |= enNormalMap; }

		if (m_pVSShader != &m_vsDefaultShader[enALL]) {
			//�J�X�^���V�F�[�_�[
			deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSShader->GetBody(), NULL, 0);
		}
		else {
			//�f�t�H���g�V�F�[�_�[
			deviceContext->VSSetShader((ID3D11VertexShader*)m_vsDefaultShader[macroind].GetBody(), NULL, 0);
		}
		if (m_pPSShader != &m_psDefaultShader[enALL]) {
			//�J�X�^���V�F�[�_�[
			deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSShader->GetBody(), NULL, 0);
		}
		else {
			//�f�t�H���g�V�F�[�_�[		
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psDefaultShader[macroind].GetBody(), NULL, 0);
		}
		break;
	}

	//�e�N�X�`��
	deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, &m_pAlbedoTex);
	if (m_pNormalTex) {
		deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, &m_pNormalTex);
	}

	//�萔�o�b�t�@
	deviceContext->UpdateSubresource(m_materialParamCB, 0, NULL, &m_materialParam, 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB);
}

}