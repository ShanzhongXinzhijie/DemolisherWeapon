#include "DWstdafx.h"
#include "CSkinModelRender.h"

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
	if (!m_isDraw) { return; }

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
		DrawLine({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

		DrawLine({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

		DrawLine({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
	}
}

void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips,
	EnFbxUpAxis fbxUpAxis,
	EnFbxCoordinateSystem fbxCoordinate) {

	if (m_isInit) { return; }

	//モデル読み込み
	m_model.Init(filePath, fbxUpAxis, fbxCoordinate);
	
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

}
}