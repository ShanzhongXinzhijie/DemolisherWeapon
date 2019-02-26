#include "DWstdafx.h"
#include "CSkybox.h"

namespace DemolisherWeapon {
namespace GameObj {

	CSkybox::CSkybox(const wchar_t* filePass, float size)
	{
		//モデル
		m_skyModel.Init(L"Preset/modelData/sky.cmo");

		//テクスチャ
		ID3D11ShaderResourceView* tex;
		DirectX::CreateDDSTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), filePass, nullptr, &tex);
		
		//シェーダ
		D3D_SHADER_MACRO macros[] = { "SKY_CUBE", "1", NULL, NULL };
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "SKY_CUBE", macros);

		//モデルにテクスチャとシェーダ設定
		m_skyModel.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(tex);
				mat->SetPS(&m_psShader);
				mat->SetLightingEnable(false);
			}
		);

		//設定
		m_skyModel.SetDrawPriority(DRAW_PRIORITY_MAX-1);
		m_skyModel.SetIsMostDepth(true);
		m_skyModel.SetIsShadowCaster(false);
		
		//大きさ
		if (size < 0.0f) { size = std::floor(GetMainCamera()->GetFar() * sqrt(2.0f)); }
		m_skyModel.SetScale({ size / 50.0f, size / 50.0f, size / 50.0f });
	}

	CSkybox::~CSkybox()
	{
	}

	void CSkybox::Update() {
		m_skyModel.SetPos(GetMainCamera()->GetPos());
	}
}
}