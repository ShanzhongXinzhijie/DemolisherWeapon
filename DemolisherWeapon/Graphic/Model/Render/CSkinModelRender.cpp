#include "DWstdafx.h"
#include "CSkinModelRender.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {
namespace GameObj {

ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCw = nullptr;
ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCCw = nullptr;
ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSNone = nullptr;

CSkinModelRender::CSkinModelRender()
{
	if (!m_mostDepthRSCw) {
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);
		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSCw);
	}
	if (!m_mostDepthRSCCw) {
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);
		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSCCw);
	}
	if (!m_mostDepthRSNone) {
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_NONE;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);
		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSNone);
	}
}

void CSkinModelRender::PreLoopUpdate() {
	if (!m_isInit) { return; }

	//旧座標の更新
	m_model.UpdateOldMatrix();
}

void CSkinModelRender::Update() {
	if (!m_isInit || m_animUpdating || !m_isEnableUpdate) { return; }

	if (!m_isUpdated) {
		m_animUpdating = true;
		m_animCon.Update();
		m_animUpdating = false;
	}
	if(!m_isUpdated || !m_isUpdatedWorldMatrix){
		UpdateWorldMatrix(m_isRefreshMode);
	}

	m_isUpdated = true;
}

void CSkinModelRender::PostUpdate() {
	if (!m_isInit) { return; }

	ImNonUpdate();
}

void CSkinModelRender::PostLoopUpdate() {
	if (!m_isInit) { return; }		
	if (!m_model.GetIsDraw()) { return; }

	if (m_isShadowCaster) {
		//シャドウマップ描画前後で実行する処理を送る
		//if (m_shadowMapPrePost) { GetGraphicsEngine().GetShadowMapRender().AddPrePostAction(m_shadowMapPrePost.get()); }
		//シャドウマップレンダーにモデル送る
		AddDrawModelToShadowMapRender(&m_model, m_priority, (m_isDrawReverse != m_isShadowDrawReverse));
	}
	if (m_isPostDraw) {
		//ポストドローレンダーにモデル送る
		GetGraphicsEngine().AddDrawModelToPostDrawRender(&m_model, m_priority, m_postDrawBlendMode, m_isDrawReverse);
	}
	else {
		//3Dモデルレンダーにモデル送る
		AddDrawModelToD3Render(&m_model, m_priority, m_isDrawReverse);
	}

	//バウンディングボックスの表示
	if (m_isDrawBoundingBox) {
		CVector3 min, max;
		m_model.GetUpdatedBoundingBox(min, max);
		DrawLine3D({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

		DrawLine3D({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

		DrawLine3D({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
	}
}

void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips,
	EnFbxUpAxis fbxUpAxis,
	EnFbxCoordinateSystem fbxCoordinate,
	bool isUseFlyweightFactory
) {

	if (m_isInit) { return; }

	//モデル読み込み
	m_model.Init(filePath, fbxUpAxis, fbxCoordinate, isUseFlyweightFactory);
	
	//アニメーションの初期化。
	if (animationClips != nullptr) {
		m_animCon.Init(
			m_model,			//アニメーションを流すスキンモデル。
			animationClips,		//アニメーションクリップの配列。
			numAnimationClips	//アニメーションクリップの数。
		);
	}

	m_isInit = true;
}

void CSkinModelRender::InitPostDraw(PostDrawModelRender::enBlendMode blendMode, bool isPMA, bool isSoftParticle, float softParticleArea) {
	m_isPostDraw = true; m_postDrawBlendMode = blendMode;

	bool oldIsSoftParticle = m_shaderSNAIsSoftParticle;

	//シェーダ読み込み
	if (!m_loadedShaderSNA || m_shaderSNAIsConvertPMA != !isPMA || m_shaderSNAIsSoftParticle != isSoftParticle) {
		m_loadedShaderSNA = true; m_shaderSNAIsConvertPMA = !isPMA; m_shaderSNAIsSoftParticle = isSoftParticle;

		//マクロ
		D3D_SHADER_MACRO macros[3] = { NULL,NULL,NULL,NULL,NULL,NULL }, macrosTex[3] = { "TEXTURE","1",NULL,NULL,NULL,NULL };
		char shaderName[64] = "DEFAULT", shaderNameTex[64] = "TEXTURE";
		//ソフトパーティクル設定
		if (m_shaderSNAIsSoftParticle) {
			macros[0].Name = "SOFT_PARTICLE";
			macros[0].Definition = "1";
			strcpy_s(shaderName, "SOFT_PARTICLE");
			macrosTex[1].Name = "SOFT_PARTICLE";
			macrosTex[1].Definition = "1";
			strcpy_s(shaderNameTex, "TEXTURE+SOFT_PARTICLE");
		}

		if (m_shaderSNAIsConvertPMA) {
			//乗算済みアルファに変換する
			m_psSozaiNoAzi.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi_ConvertToPMA", Shader::EnType::PS, shaderName, macros);
			m_psSozaiNoAziTex.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi_ConvertToPMA", Shader::EnType::PS, shaderNameTex, macrosTex);
		}
		else {
			//通常
			m_psSozaiNoAzi.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi", Shader::EnType::PS, shaderName, macros);
			m_psSozaiNoAziTex.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi", Shader::EnType::PS, shaderNameTex, macrosTex);
		}
	}
	//シェーダ設定
	GetSkinModel().FindMaterialSetting(
		[&](MaterialSetting* mat) {
			//ピクセルシェーダ
			if (mat->GetAlbedoTexture()) {
				mat->SetPS(&m_psSozaiNoAziTex);//テクスチャあり
			}
			else {
				mat->SetPS(&m_psSozaiNoAzi);//テクスチャなし
			}
		}
	);
	//ソフトパーティクル設定
	if (isSoftParticle != oldIsSoftParticle) {
		if (isSoftParticle) {
			//デプスバッファを設定
			GetSkinModel().SetPreDrawFunction(
				L"DW_SetViewPosTexture",
				[&](SkinModel*) {
					GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(enSkinModelSRVReg_ViewPosTexture, 1, &GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));
				}
			);
			//デプスバッファを解除
			GetSkinModel().SetPostDrawFunction(
				L"DW_RemoveViewPosTexture",
				[&](SkinModel*) {
					ID3D11ShaderResourceView* view[] = { NULL };
					GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(enSkinModelSRVReg_ViewPosTexture, 1, view);
				}
			);
		}
		else {
			//設定解除
			GetSkinModel().ErasePreDrawFunction(L"DW_SetViewPosTexture");
			GetSkinModel().ErasePostDrawFunction(L"DW_RemoveViewPosTexture");
		}
	}
	//ソフトパーティクル発生距離設定
	if (softParticleArea < 0.0f) {
		//デフォルト
		GetSkinModel().SetSoftParticleArea(50.0f*(GetEngine().GetDistanceScale() / 3.5f));		
	}
	else {
		GetSkinModel().SetSoftParticleArea(softParticleArea);
	}
}

}
}