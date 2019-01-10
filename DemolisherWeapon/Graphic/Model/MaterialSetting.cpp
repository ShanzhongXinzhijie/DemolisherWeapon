#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

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