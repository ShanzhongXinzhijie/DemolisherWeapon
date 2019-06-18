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

	//アニメーションの更新
	int i = 0;
	for (auto& anim : m_animations) {
		anim.Update(m_animPlaySpeedSec[i]*updateTime);
		i++;
	}

	//グローバルポーズをスケルトンに反映させていく。
	int numBone = m_skeleton->GetNumBones();
	for (int boneNo = 0; boneNo < numBone; boneNo++) {//すべてのボーンに実行

		CVector3 integratedTrans, integratedScale = 1.0f;
		CQuaternion integratedRot;

		//アニメーションの分だけ実行
		for (auto& anim : m_animations) {
			//ボーンの状態を取得
			CVector3 trans, scr;
			CQuaternion rot;
			anim.GetGlobalPose(boneNo, trans, rot, scr);
			//ブレンド
			integratedTrans += trans;
			if (&anim == &m_animations[0]) {
				integratedRot = rot;
			}
			else {
				integratedRot.Slerp(0.5f, integratedRot, rot);//仮
			}
			integratedScale *= scr;
		}

		//全部を合成して、ボーン行列を作成。
		CMatrix boneMatrix, rotMatrix;
		boneMatrix.MakeScaling(integratedScale);			//拡大行列を作成
		rotMatrix.MakeRotationFromQuaternion(integratedRot);//回転行列を作成
		boneMatrix.Mul(boneMatrix, rotMatrix);				//拡大×回転
		boneMatrix.SetTranslation(integratedTrans);			//平行移動を適用

		//ボーンに設定
		m_skeleton->SetBoneLocalMatrix(
			boneNo,
			boneMatrix
		);
	}
	
}

}