#include "DWstdafx.h"
#include "CSkybox.h"

namespace DemolisherWeapon {
namespace GameObj {

	CSkybox::CSkybox(const wchar_t* filePass, float size, bool isSetAmbientCube) {
		Init(filePass, size, isSetAmbientCube);
	}
	
	void CSkybox::Init(const wchar_t* filePass, float size, bool isSetAmbientCube)
	{
		if (m_isInit) { return; }

		//テクスチャ
		ID3D11ShaderResourceView* tex = nullptr;
		HRESULT hr = DirectX::CreateDDSTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), filePass, nullptr, &tex);
		if (FAILED(hr)) {
			Error::Box("CSkyboxのテクスチャ読み込みに失敗しました");
			return;
		}

		//モデル
		m_skyModel.Init(L"Preset/modelData/sky.cmo");
		
		//シェーダ
		D3D_SHADER_MACRO macros[] = { "SKY_CUBE", "1", "MOTIONBLUR", "1", NULL, NULL };
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "SKY_CUBE", macros);

		//モデルにテクスチャとシェーダ設定
		m_skyModel.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(tex);
				mat->SetPS(&m_psShader);
				mat->SetLightingEnable(false);
			}
		);

		//環境キューブマップを設定
		if (isSetAmbientCube) {
			SetAmbientCubeMap(tex, CVector3::One());
		}

		if (tex) { tex->Release(); }

		//設定
		m_skyModel.SetDrawPriority(DRAW_PRIORITY_MAX-1);
		m_skyModel.SetIsMostDepth(true);
		m_skyModel.SetIsShadowCaster(false);
		
		//大きさ
		if (size < 0.0f) { size = std::floor(GetMainCamera()->GetFar() * (1.0f/sqrt(3.0f))); }
		m_skyModel.SetScale({ size / 50.0f, size / 50.0f, size / 50.0f });

		m_isInit = true;
	}

	CSkybox::~CSkybox()
	{
	}

	void CSkybox::Update() {
		if (!m_isInit) { return; }
		m_skyModel.SetPos(GetMainCamera()->GetPos());
	}
}
}