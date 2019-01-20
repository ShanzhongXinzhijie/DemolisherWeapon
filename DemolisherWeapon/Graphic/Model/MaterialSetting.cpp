#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;
		SetDefaultPS();
		SetDefaultAlbedoTexture();

		//設定のコピー
		SetMatrialName(modeleffect->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(modeleffect->GetDefaultMaterialSetting().GetMaterialParam());
		SetPS(modeleffect->GetDefaultMaterialSetting().GetPS());
		SetAlbedoTexture(modeleffect->GetDefaultMaterialSetting().GetAlbedoTexture());
		SetIsMotionBlur(modeleffect->GetDefaultMaterialSetting().GetIsMotionBlur());
	}

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