#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;//���ƂȂ郂�f���G�t�F�N�g���L�^

		//������
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
		SetTranslucentTexture(modeleffect->GetDefaultMaterialSetting().GetTranslucentTexture());

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
	void MaterialSetting::SetTriPlanarMappingPS(bool isYOnly) {
		m_pPSShader = m_isInit->GetTriPlanarMappingPS(isYOnly);
	}

	//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultAlbedoTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultAlbedoTexture();
		SetAlbedoTexture(DT);
	}
	//�m�[�}���}�b�v���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultNormalTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultNormalTexture();
		SetNormalTexture(DT);
	}
	//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultLightingTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultLightingTexture();
		SetLightingTexture(DT);
	}
}