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

	//���f���ǂݍ���
	m_model.Init(filePath, fbxUpAxis);
	m_model.SetMotionBlurFlag(true);

	//�A�j���[�V�����̏������B
	if (animationClips != nullptr) {
		m_animCon.Init(
			m_model,			//�A�j���[�V�����𗬂��X�L�����f���B
			animationClips,		//�A�j���[�V�����N���b�v�̔z��B
			numAnimationClips	//�A�j���[�V�����N���b�v�̐��B
		);
	}
}

}
}