#include "DWstdafx.h"
#include "AnimationController.h"

namespace DemolisherWeapon {

AnimationController::AnimationController()
{
}


AnimationController::~AnimationController()
{
}

int AnimationController::AddAnimation() {
	m_animations.emplace_back();
	m_animations.back().Init(*m_skinModel, m_animationClips, m_numAnimClip);
	m_animPlaySpeedSec.emplace_back(1.0f);

	return (int)m_animations.size()-1;
}

void AnimationController::Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip) {
	if (animClipList == nullptr) {
#ifndef DW_MASTER
		char message[256];
		strcpy_s(message, "animClipListがNULLです。(AnimationController)\n");
		OutputDebugStringA(message);
		//止める。
		std::abort();
#endif
	}

	m_skinModel = &skinModel;
	m_skeleton = &skinModel.GetSkeleton();
	m_numAnimClip = numAnimClip;
	m_animationClips = animClipList;

	AddAnimation();//とりまいっこつくる
}

void AnimationController::Update() {
	Update(GetDeltaTimeSec());
}

void AnimationController::Update(float updateTime) {

	//アニメーションある?
	if (m_animations.size() == 0) { return; }

	int i = 0;
	for (auto& anim : m_animations) {
		anim.Update(m_animPlaySpeedSec[i]*updateTime);
		i++;
	}

	//グローバルポーズをスケルトンに反映させていく。
	int numBone = m_skeleton->GetNumBones();
	for (int boneNo = 0; boneNo < numBone; boneNo++) {

		CMatrix scaleMatrix;
		CMatrix transMat;
		CMatrix rotMatrix;

		for (auto& anim : m_animations) {
			CVector3 trans, scr;
			CQuaternion rot;
			anim.GetGlobalPose( boneNo, trans, rot, scr);

			//拡大行列を作成。
			scaleMatrix.MakeScaling(scr);
			//回転行列を作成。
			rotMatrix.MakeRotationFromQuaternion(rot);
			//平行移動行列を作成。
			transMat.MakeTranslation(trans);
		}

		//全部を合成して、ボーン行列を作成。
		CMatrix boneMatrix;
		boneMatrix.Mul(scaleMatrix, rotMatrix);
		boneMatrix.Mul(boneMatrix, transMat);

		m_skeleton->SetBoneLocalMatrix(
			boneNo,
			boneMatrix
		);
	}
	
}

}