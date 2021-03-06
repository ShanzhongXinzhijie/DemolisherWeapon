#include "DWstdafx.h"
#include "CSkybox.h"

namespace DemolisherWeapon {
namespace GameObj {

	CSkybox::CSkybox(const wchar_t* filePass, float size, bool isSetAmbientCube, const CVector3& ambientScale) {
		Init(filePass, size, isSetAmbientCube);
	}
	
	void CSkybox::Init(const wchar_t* filePass, float size, bool isSetAmbientCube, const CVector3& ambientScale)
	{
		if (m_isInit) { return; }

		//テクスチャ
		ID3D11ShaderResourceView* texSRV = nullptr;
		TextueData texData = CreateTexture(filePass);
		if (!texData.isLoaded()) {
			Error::Box("CSkyboxのテクスチャ読み込みに失敗しました");
			return;
		}
		texSRV = texData.textureView.Get(); texSRV->AddRef();

		//モデル
		m_skyModel.Init(L"Preset/modelData/sky.cmo");
		
		//シェーダ
		D3D_SHADER_MACRO macros[] = { "SKY_CUBE", "1", "MOTIONBLUR", "1", NULL, NULL };
		m_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "SKY_CUBE", macros);
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "SKY_CUBE", macros);

		//モデルにテクスチャとシェーダ設定
		m_skyModel.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(texSRV);
				mat->SetAlbedoScale(ambientScale);
				mat->SetVS(&m_vsShader);
				mat->SetPS(&m_psShader);
				mat->SetLightingEnable(false);
			}
		);

		//環境キューブマップを設定
		if (isSetAmbientCube) {
			SetAmbientCubeMap(texSRV, ambientScale);
		}

		if (texSRV) { texSRV->Release(); }

		//設定
		m_skyModel.SetDrawPriority(DRAW_PRIORITY_MAX-1);
		m_skyModel.SetIsMostDepth(true);
		m_skyModel.SetIsShadowCaster(false);
		
		//大きさ
		//TODO 複数カメラを使う場合、現状だとFarを合わせる必要がある
		if (size < 0.0f) { size = std::floor(GetMainCamera()->GetFar() * (1.0f/sqrt(3.0f))); }
		m_skyModel.SetScale({ size / 50.0f, size / 50.0f, size / 50.0f });

		//描画前処理を設定
		m_skyModel.GetSkinModel().SetPreCullingFunction(
			[&](SkinModel* model) {
				m_skyModel.SetPos(GetMainCamera()->GetPos());
				m_skyModel.RefreshWorldMatrix();
			}
		);

		m_isInit = true;
	}
}
}