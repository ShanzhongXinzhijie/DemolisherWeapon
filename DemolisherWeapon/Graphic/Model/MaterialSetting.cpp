#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultAlbedoTexture();

		//�ݒ�̃R�s�[
		SetMatrialName(modeleffect->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(modeleffect->GetDefaultMaterialSetting().GetMaterialParam());

		SetVS(modeleffect->GetDefaultMaterialSetting().GetVS());
		SetVSZ(modeleffect->GetDefaultMaterialSetting().GetVSZ());
		SetPS(modeleffect->GetDefaultMaterialSetting().GetPS());

		SetAlbedoTexture(modeleffect->GetDefaultMaterialSetting().GetAlbedoTexture());
		SetNormalTexture(modeleffect->GetDefaultMaterialSetting().GetNormalTexture());
		SetLightingTexture(modeleffect->GetDefaultMaterialSetting().GetLightingTexture());

		SetIsMotionBlur(modeleffect->GetDefaultMaterialSetting().GetIsMotionBlur());
		SetIsUseTexZShader(modeleffect->GetDefaultMaterialSetting().GetIsUseTexZShader());
	}

	//�V�F�[�_���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultVS() {
		m_pVSShader = m_isInit->GetDefaultVS();
	}
	void MaterialSetting::SetDefaultVSZ() {
		m_pVSZShader = m_isInit->GetDefaultVSZ();
	}
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
		else {
			SetAlbedoScale(CVector4::One());//�A���x�h�X�P�[����������
		}
		m_pAlbedoTex = DT;
		if (m_pAlbedoTex) {
			m_pAlbedoTex->AddRef();
		}
	}

}