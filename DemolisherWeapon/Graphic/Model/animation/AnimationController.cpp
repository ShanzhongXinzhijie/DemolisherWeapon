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
		strcpy_s(message, "animClipList��NULL�ł��B(AnimationController)\n");
		OutputDebugStringA(message);
		//�~�߂�B
		std::abort();
#endif
	}

	m_skinModel = &skinModel;
	m_skeleton = &skinModel.GetSkeleton();
	m_numAnimClip = numAnimClip;
	m_animationClips = animClipList;

	AddAnimation();//�Ƃ�܂���������
}

void AnimationController::Update() {

	//�A�j���[�V��������?
	if (m_animations.size() == 0) { return; }

	int i = 0;
	for (auto& anim : m_animations) {
		anim.Update(m_animPlaySpeedSec[i]*(1.0f / GetStandardFrameRate()));
		i++;
	}

	//�O���[�o���|�[�Y���X�P���g���ɔ��f�����Ă����B
	int numBone = m_skeleton->GetNumBones();
	for (int boneNo = 0; boneNo < numBone; boneNo++) {

		CMatrix scaleMatrix;
		CMatrix transMat;
		CMatrix rotMatrix;

		for (auto& anim : m_animations) {
			CVector3 trans, scr;
			CQuaternion rot;
			anim.GetGlobalPose( boneNo, trans, rot, scr);

			//�g��s����쐬�B
			scaleMatrix.MakeScaling(scr);
			//��]�s����쐬�B
			rotMatrix.MakeRotationFromQuaternion(rot);
			//���s�ړ��s����쐬�B
			transMat.MakeTranslation(trans);
		}

		//�S�����������āA�{�[���s����쐬�B
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