#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;
		SetDefaultPS();
		SetDefaultAlbedoTexture();

		//�ݒ�̃R�s�[
		SetMatrialName(modeleffect->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(modeleffect->GetDefaultMaterialSetting().GetMaterialParam());
		SetPS(modeleffect->GetDefaultMaterialSetting().GetPS());
		SetAlbedoTexture(modeleffect->GetDefaultMaterialSetting().GetAlbedoTexture());
		SetIsMotionBlur(modeleffect->GetDefaultMaterialSetting().GetIsMotionBlur());
	}

	//�V�F�[�_���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultPS() {
		m_pPSShader = m_isInit->GetDefaultPS();
	}

	//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultAlbedoTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultAlbedoTexture();

		if (m_pAlbedoTex == DT) { return; }//���Ƀf�t�H���g�e�N�X�`��

		if (m_pAlbedoTex) { 
			m_pAlbedoTex->Release();
		}
		m_pAlbedoTex = DT;
		m_pAlbedoTex->AddRef();
	}

}