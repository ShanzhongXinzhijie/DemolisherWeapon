#include "DWstdafx.h"
#include "CBillboard.h"
#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {	

	void CBillboard::Init(const wchar_t* fileName, int instancingNum, bool isBillboardShader) {
		//ファクトリでテクスチャ読み込み
		ID3D11ShaderResourceView* tex = nullptr;
		TextureFactory::GetInstance().Load(fileName, nullptr, &tex);

		//ビルボード初期化
		Init(tex, instancingNum, fileName, isBillboardShader);

		//テクスチャ、リリース
		if (tex) { tex->Release(); }
	}
	void CBillboard::Init(ID3D11ShaderResourceView* srv, int instancingNum, const wchar_t* identifiers, bool isBillboardShader) {
		//インスタンシング描画か?
		m_isIns = instancingNum > 1 && identifiers ? true : false;

		//ビルボードモデル読み込み
		if (m_isIns) {
			m_insModel = std::make_unique<GameObj::CInstancingModelRender>();
			m_insModel->Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, &identifiers);
		}
		else {
			m_model = std::make_unique<GameObj::CSkinModelRender>();
			m_model->Init(L"Preset/modelData/billboard.cmo");
		}

		if (isBillboardShader) {
			//シェーダ読み込み
			if (m_isIns) {
				//インスタンシング用シェーダ
				D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
				m_vsShader.Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
				m_vsZShader.Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
			}
			else {
				D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
				m_vsShader.Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
				m_vsZShader.Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
			}
		}

		//CSkinModelRender
		GameObj::CSkinModelRender* modelPtr = m_model.get();
		if (m_isIns) { modelPtr = &m_insModel->GetInstancingModel()->GetModelRender(); }

		//いろいろ設定
		modelPtr->GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(srv);
				mat->SetIsUseTexZShader(true);//Z値出力シェーダでテクスチャを使用
				if (isBillboardShader) {
					mat->SetVS(&m_vsShader);
					mat->SetVSZ(&m_vsZShader);
				}
			}
		);

		//バウンディングボックスの設定
		float size = sqrt(CMath::Square(0.5f) + CMath::Square(0.5f));
		modelPtr->GetSkinModel().SetBoundingBox({ -size,-size,-size }, { size,size,size });

		//シャドウマップの描画時に面を反転させない
		modelPtr->SetIsShadowDrawReverse(false);

		//初期化完了
		m_isInit = true;
	}

}