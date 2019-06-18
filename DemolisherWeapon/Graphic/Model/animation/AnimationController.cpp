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
	Update(GetDeltaTimeSec());
}

void AnimationController::Update(float updateTime) {
	//�A�j���[�V��������?
	if (m_animations.size() == 0) { return; }

	//�A�j���[�V�����̍X�V
	int i = 0;
	for (auto& anim : m_animations) {
		anim.Update(m_animPlaySpeedSec[i]*updateTime);
		i++;
	}

	//�O���[�o���|�[�Y���X�P���g���ɔ��f�����Ă����B
	int numBone = m_skeleton->GetNumBones();
	for (int boneNo = 0; boneNo < numBone; boneNo++) {//���ׂẴ{�[���Ɏ��s

		CVector3 integratedTrans, integratedScale = 1.0f;
		CQuaternion integratedRot;

		//�A�j���[�V�����̕��������s
		for (auto& anim : m_animations) {
			//�{�[���̏�Ԃ��擾
			CVector3 trans, scr;
			CQuaternion rot;
			anim.GetGlobalPose(boneNo, trans, rot, scr);
			//�u�����h
			integratedTrans += trans;
			if (&anim == &m_animations[0]) {
				integratedRot = rot;
			}
			else {
				integratedRot.Slerp(0.5f, integratedRot, rot);//��
			}
			integratedScale *= scr;
		}

		//�S�����������āA�{�[���s����쐬�B
		CMatrix boneMatrix, rotMatrix;
		boneMatrix.MakeScaling(integratedScale);			//�g��s����쐬
		rotMatrix.MakeRotationFromQuaternion(integratedRot);//��]�s����쐬
		boneMatrix.Mul(boneMatrix, rotMatrix);				//�g��~��]
		boneMatrix.SetTranslation(integratedTrans);			//���s�ړ���K�p

		//�{�[���ɐݒ�
		m_skeleton->SetBoneLocalMatrix(
			boneNo,
			boneMatrix
		);
	}
	
}

}