#include "DWstdafx.h"
#include "MaterialSetting.h"
#include "Graphic/Factory/TextureFactory.h"
#include "Graphic/Model/Model.h"

namespace DemolisherWeapon {

	void MaterialData::Init(bool isSkining, std::wstring_view name) {
		m_isSkining = isSkining;
		
		//名前設定
		GetDefaultMaterialSetting().SetMatrialName(name.data());

		//シェーダ作成
		{
			//頂点シェーダ
			if (m_isSkining) {
				//マクロごとに頂点シェーダを作成
				m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
				//Z値描画シェーダを作成
				m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);
			}
			else {
				//マクロごとに頂点シェーダを作成
				m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
				//Z値描画シェーダを作成
				m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);
			}

			//マクロごとにピクセルシェーダを作成
			m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
			m_psTriPlanarMapShader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
			D3D_SHADER_MACRO macrosYOnly[] = { "Y_ONLY", "1", NULL, NULL };
			m_psTriPlanarMapShaderYOnly.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS, "Y_ONLY", macrosYOnly);

			//マクロごとにZ値描画ピクセルシェーダを作成
			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
			m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
			m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		}

		//マテリアル設定(m_defaultMaterialSetting)を初期化してやる
		m_defaultMaterialSetting.Init(this);

		//使用するマテリアル設定をセット
		m_ptrUseMaterialSetting = &m_defaultMaterialSetting;

		//マテリアルパラメーターの定数バッファ
		m_materialParamCB.Init(sizeof(MaterialParam));
	}
	void MaterialData::InitAlbedoTexture(std::wstring_view path) {
		//アルベド
		const TextueData* return_textureData;
		if (TextureFactory::GetInstance().Load(path.data(), &return_textureData, true)) {
			//テクスチャで設定
			m_defaultMaterialSetting.SetAlbedoTexture(*return_textureData);
		}
	}
	void MaterialData::InitAlbedoColor(const CVector3& rgb) {
		//ディフューズカラー(数値)で設定
		m_defaultMaterialSetting.SetAlbedoScale({ rgb.x,rgb.y,rgb.z, 1.0f });
	}
	void MaterialData::InitNormalTexture(std::wstring_view path) {
		//ノーマル
		const TextueData* return_textureData;
		if (TextureFactory::GetInstance().Load(path.data(), &return_textureData, true))
		{
			//テクスチャで設定			
			m_defaultMaterialSetting.SetNormalTexture(*return_textureData);
		}
	}
	void MaterialData::InitLightingTexture(std::wstring_view path) {
		//ライティングパラメータ
		const TextueData* return_textureData;
		if (TextureFactory::GetInstance().Load(path.data(), &return_textureData, true))
		{
			//テクスチャで設定
			m_defaultMaterialSetting.SetLightingTexture(*return_textureData);
		}
	}	

	/*void MaterialSetting::Init(ModelEffect* modeleffect) {
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
	}*/

	void MaterialSetting::Init(MaterialData* materialData) {
		m_isInit = materialData;//元となるモデルエフェクトを記録

		//初期化
		//一番最初に作られる親のマテリアルセッティング初期化のため
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultPSZ();
		SetDefaultAlbedoTexture();
		SetDefaultNormalTexture();
		SetDefaultLightingTexture();

		//設定のコピー
		SetMatrialName(materialData->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(materialData->GetDefaultMaterialSetting().GetMaterialParam());

		SetVS(materialData->GetDefaultMaterialSetting().GetVS());
		SetVSZ(materialData->GetDefaultMaterialSetting().GetVSZ());
		SetPS(materialData->GetDefaultMaterialSetting().GetPS());
		SetPSZ(materialData->GetDefaultMaterialSetting().GetPSZ());

		SetAlbedoTexture(materialData->GetDefaultMaterialSetting().GetAlbedoTextureData());
		SetNormalTexture(materialData->GetDefaultMaterialSetting().GetNormalTextureData());
		SetLightingTexture(materialData->GetDefaultMaterialSetting().GetLightingTextureData());
		SetTranslucentTexture(materialData->GetDefaultMaterialSetting().GetTranslucentTextureData());

		SetIsMotionBlur(materialData->GetDefaultMaterialSetting().GetIsMotionBlur());
		SetIsUseTexZShader(materialData->GetDefaultMaterialSetting().GetIsUseTexZShader());
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
		auto DT = m_isInit->GetDefaultAlbedoTexture();
		SetAlbedoTexture(DT);
	}
	//ノーマルマップをデフォに戻す
	void MaterialSetting::SetDefaultNormalTexture() {
		auto DT = m_isInit->GetDefaultNormalTexture();
		SetNormalTexture(DT);
	}
	//ライティングパラメータマップをデフォに戻す
	void MaterialSetting::SetDefaultLightingTexture() {
		auto DT = m_isInit->GetDefaultLightingTexture();
		SetLightingTexture(DT);
	}

	//スキンモデルか取得
	bool MaterialSetting::GetIsSkining() const{
		return m_isInit->GetIsSkining();
	}

}