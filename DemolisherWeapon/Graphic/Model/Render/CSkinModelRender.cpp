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

void CSkinModelRender::PreLoopUpdate() {
	if (!m_isInit) { return; }

	//�����W�̍X�V
	m_model.UpdateOldMatrix();
}

void CSkinModelRender::Update() {
	if (!m_isInit) { return; }

	if (!m_isUpdated) {
		m_animCon.Update();
	}
	if(!m_isUpdated || !m_isUpdatedWorldMatrix){
		UpdateWorldMatrix();
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
	if (m_isShadowCaster) { AddDrawModelToShadowMapRender(&m_model); }
	AddDrawModelToD3Render(&m_model);
}

void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips,
	EnFbxUpAxis fbxUpAxis,
	EnFbxCoordinateSystem fbxCoordinate) {

	if (m_isInit) { return; }

	//���f���ǂݍ���
	m_model.Init(filePath, fbxUpAxis, fbxCoordinate);
	m_model.SetMotionBlurFlag(true);

	//�A�j���[�V�����̏������B
	if (animationClips != nullptr) {
		m_animCon.Init(
			m_model,			//�A�j���[�V�����𗬂��X�L�����f���B
			animationClips,		//�A�j���[�V�����N���b�v�̔z��B
			numAnimationClips	//�A�j���[�V�����N���b�v�̐��B
		);
	}

	m_isInit = true;
}

}
}