#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultAlbedoTexture();
		SetDefaultNormalTexture();
		SetDefaultLightingTexture();

		//設定のコピー
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

	//シェーダをデフォに戻す
	void MaterialSetting::SetDefaultVS() {
		m_pVSShader = m_isInit->GetDefaultVS();
	}
	void MaterialSetting::SetDefaultVSZ() {
		m_pVSZShader = m_isInit->GetDefaultVSZ();
	}
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
		else {
			SetAlbedoScale(CVector4::One());//アルベドスケールを初期化
		}
		m_pAlbedoTex = DT;
		if (m_pAlbedoTex) {
			m_pAlbedoTex->AddRef();
		}
	}
	//ノーマルマップをデフォに戻す
	void MaterialSetting::SetDefaultNormalTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultNormalTexture();

		if (m_pNormalTex == DT) { return; }//既にデフォルトテクスチャ

		if (m_pNormalTex) {
			m_pNormalTex->Release();
		}
		m_pNormalTex = DT;
		if (m_pNormalTex) {
			m_pNormalTex->AddRef();
		}
	}
	//ライティングパラメータマップをデフォに戻す
	void MaterialSetting::SetDefaultLightingTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultLightingTexture();

		if (m_pLightingTex == DT) { return; }//既にデフォルトテクスチャ

		if (m_pLightingTex) {
			m_pLightingTex->Release();
		}
		else {
			//初期化(これらのパラメータはテクスチャにかけるスケールとして使う)
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