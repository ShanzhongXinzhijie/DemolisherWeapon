#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	//シェーダをデフォに戻す
	void MaterialSetting::SetDefaultPS() {
		m_pPSShader = m_isInit->GetDefaultPS();
	}

	//アルベドテクスチャをデフォに戻す
	void MaterialSetting::SetDefaultAlbedoTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultAlbedoTexture();

		if (m_pAlbedoTex == DT) { return; }//既にデフォルトテクスチャ

		if (m_pAlbedoTex) { 
			m_pAlbedoTex->Release();
		}
		m_pAlbedoTex = DT;
		m_pAlbedoTex->AddRef();
	}

}