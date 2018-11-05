#include "DWstdafx.h"
#include "CSkinModelRender.h"

namespace DemolisherWeapon {
namespace GameObj {

CSkinModelRender::CSkinModelRender()
{
}
CSkinModelRender::~CSkinModelRender()
{
}

void CSkinModelRender::Update() {
	m_animCon.Update();
	UpdateWorldMatrix();
}

void CSkinModelRender::PostLoopUpdate() {
	if (!m_isDraw) { return; }
	if (m_isShadowCaster) { AddDrawModelToShadowMapRender(&m_model); }
	AddDrawModelToD3Render(&m_model);
}

void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips,
	EnFbxUpAxis fbxUpAxis) {

	//モデル読み込み
	m_model.Init(filePath, fbxUpAxis);
	m_model.SetMotionBlurFlag(true);

	//アニメーションの初期化。
	if (animationClips != nullptr) {
		m_animCon.Init(
			m_model,			//アニメーションを流すスキンモデル。
			animationClips,		//アニメーションクリップの配列。
			numAnimationClips	//アニメーションクリップの数。
		);
	}
}

}
}