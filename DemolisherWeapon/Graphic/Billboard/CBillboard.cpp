#include "DWstdafx.h"
#include "CBillboard.h"
#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {	

	void CBillboard::Init(const wchar_t* fileName, int instancingNum, bool isBillboardShader) {
		//ファクトリでテクスチャ読み込み
		ID3D11ShaderResourceView* tex = nullptr;
		const TextureFactory::TextueData* texdata = nullptr;
		if (!TextureFactory::GetInstance().Load(fileName, nullptr, &tex, &texdata)) {
			//失敗
			return;
		}

		//アスペクト比算出
		m_aspect = (float)texdata->width / texdata->height;

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
			m_insModel = std::make_unique<GameObj::CInstancingModelRender>(m_isRegister);
			std::wstring_view string[1]; string[0] = identifiers;
			m_insModel->Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, string);
		}
		else {
			m_model = std::make_unique<GameObj::CSkinModelRender>(m_isRegister);
			m_model->Init(L"Preset/modelData/billboard.cmo");
		}

		if (!m_s_isShaderLoaded && isBillboardShader) {
			//シェーダ読み込み
			{
				//インスタンシング用シェーダ
				D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enInstancing].Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
				m_s_vsZShader[enInstancing].Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
			}
			{
				D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enNormal].Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
				m_s_vsZShader[enNormal].Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
			}
			m_s_isShaderLoaded = true;
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
					mat->SetVS(&m_s_vsShader[m_isIns ? enInstancing : enNormal]);
					mat->SetVSZ(&m_s_vsZShader[m_isIns ? enInstancing : enNormal]);
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