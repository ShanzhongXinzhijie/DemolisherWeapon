#include "DWstdafx.h"
#include "CSkinModelRender.h"

#define DEPTH_BIAS_D32_FLOAT(d) (d/(1/pow(2,23))) 

namespace DemolisherWeapon {
namespace GameObj {

ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCw = nullptr;
ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCCw = nullptr;

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
}
CSkinModelRender::~CSkinModelRender()
{
}

void CSkinModelRender::PreLoopUpdate() {
	if (!m_isInit) { return; }

	//旧座標の更新
	m_model.UpdateOldMatrix();
}

void CSkinModelRender::Update() {
	if (!m_isInit) { return; }

	if (!m_isUpdated && !m_animUpdating) {
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

	if (m_isShadowCaster) { AddDrawModelToShadowMapRender(&m_model, m_priority); }

	AddDrawModelToD3Render(&m_model, m_priority);
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