#include "DWstdafx.h"
#include "MaterialSetting.h"

namespace DemolisherWeapon {

	void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;//元となるモデルエフェクトを記録

		//初期化
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultPSZ();
		SetDefaultAlbedoTexture();
		SetDefaultNormalTexture();
		SetDefaultLightingTexture();

		//設定のコピー
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
	void MaterialSetting::SetDefaultPSZ() {
		m_pPSZShader = m_isInit->GetDefaultPSZ();
	}
	//TriPlanarMapping用のシェーダを設定
	void MaterialSetting::SetTriPlanarMappingPS(bool isYOnly) {
		m_pPSShader = m_isInit->GetTriPlanarMappingPS(isYOnly);
	}

	//アルベドテクスチャをデフォに戻す
	void MaterialSetting::SetDefaultAlbedoTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultAlbedoTexture();
		SetAlbedoTexture(DT);
	}
	//ノーマルマップをデフォに戻す
	void MaterialSetting::SetDefaultNormalTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultNormalTexture();
		SetNormalTexture(DT);
	}
	//ライティングパラメータマップをデフォに戻す
	void MaterialSetting::SetDefaultLightingTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultLightingTexture();
		SetLightingTexture(DT);
	}
}