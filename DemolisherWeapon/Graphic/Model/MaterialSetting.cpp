#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultPSZ();
		SetDefaultAlbedoTexture();
		SetDefaultNormalTexture();
		SetDefaultLightingTexture();

		//�ݒ�̃R�s�[
		SetMatrialName(modeleffect->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(modeleffect->GetDefaultMaterialSetting().GetMaterialParam());

		SetVS(modeleffect->GetDefaultMaterialSetting().GetVS());
		SetVSZ(modeleffect->GetDefaultMaterialSetting().GetVSZ());
		SetPS(modeleffect->GetDefaultMaterialSetting().GetPS());
		SetPSZ(modeleffect->GetDefaultMaterialSetting().GetPSZ());

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
	void MaterialSetting::SetDefaultPSZ() {
		m_pPSZShader = m_isInit->GetDefaultPSZ();
	}
	//TriPlanarMapping�p�̃V�F�[�_��ݒ�
	void MaterialSetting::SetTriPlanarMappingPS() {
		m_pPSShader = m_isInit->GetTriPlanarMappingPS();
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
	//�m�[�}���}�b�v���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultNormalTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultNormalTexture();

		if (m_pNormalTex == DT) { return; }//���Ƀf�t�H���g�e�N�X�`��

		if (m_pNormalTex) {
			m_pNormalTex->Release();
		}
		m_pNormalTex = DT;
		if (m_pNormalTex) {
			m_pNormalTex->AddRef();
		}
	}
	//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultLightingTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultLightingTexture();

		if (m_pLightingTex == DT) { return; }//���Ƀf�t�H���g�e�N�X�`��

		if (m_pLightingTex) {
			m_pLightingTex->Release();
		}
		else {
			//������(�����̃p�����[�^�̓e�N�X�`���ɂ�����X�P�[���Ƃ��Ďg��)
			SetEmissive(1.0f);
			SetMetallic(1.0f);
			SetShininess(1.0f);
		}
		m_pLightingTex = DT;
		if (m_pLightingTex) {
			m_pLightingTex->AddRef();
		}
	}
}