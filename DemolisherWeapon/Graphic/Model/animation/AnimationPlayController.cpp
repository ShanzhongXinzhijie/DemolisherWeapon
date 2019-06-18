/*!
 * @brief	�A�j���[�V�����Đ��R���g���[���B
 */

#include "DWstdafx.h"
#include "AnimationPlayController.h"
#include "Animation.h"
#include "AnimationClip.h"
#include "Graphic/Model/skeleton.h"


namespace DemolisherWeapon {
	
void AnimationPlayController::Init(Skeleton* skeleton)
{
	int numBones = skeleton->GetNumBones();
	//�{�[���s����o�V�b�Ɗm�ہB
	m_boneMatrix.resize(numBones);
}
	
void AnimationPlayController::InvokeAnimationEvent(Animation* animation, bool isPlayEvent)
{
	auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		//�A�j���[�V�����̋N�����Ԃ��߂��Ă��銎�A�܂��C�x���g�N�����Ă��Ȃ��B
		if (m_time > animEventArray[i].GetInvokeTime() && m_animEventedCnt <= i){//animEventArray[i].IsInvoked() == false) {
			m_animEventedCnt++;	//animEventArray[i].SetInvokedFlag(true);
			if (isPlayEvent) {
				animation->NotifyAnimationEventToListener(
					m_animationClip->GetName(), animEventArray[i].GetEventName()
				);
			}
		}
	}
}
	
void AnimationPlayController::StartLoop()
{
	m_currentKeyFrameNo = 0;
	m_time = 0.0f;

	m_animEventedCnt = 0;
	//�A�j���[�V�����C�x���g�����ׂĖ������ɂ���B
	/*auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		animEventArray[i].SetInvokedFlag(false);
	}*/
}
void AnimationPlayController::Update(float deltaTime, Animation* animation, bool isPlayAnimEvent)
{
	if(m_animationClip == nullptr){		
		return ;
	}
	const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();
	m_time += deltaTime;

	//�A�j���[�V�����C�x���g�̔����B
	InvokeAnimationEvent(animation, isPlayAnimEvent);

	//�⊮���Ԃ��i�߂Ă����B
	m_interpolateTime = min(m_interpolateEndTime, m_interpolateTime + deltaTime);
	while (true) {
		if (m_currentKeyFrameNo >= (int)topBoneKeyFrameList.size()) {
			//�I�[�܂ōs�����B
			if (m_animationClip->IsLoop()) {
				float amariTime = m_time - topBoneKeyFrameList.back()->time;

				//���[�v�B
				StartLoop();

				m_time = amariTime;
				InvokeAnimationEvent(animation, isPlayAnimEvent);
			}
			else {
				//�����V���b�g�Đ��B
				m_currentKeyFrameNo--;
				m_isPlaying = false;	//�Đ��I���B
				break;
			}
		}
		if (topBoneKeyFrameList.at(m_currentKeyFrameNo)->time >= m_time) {
			//�I���B
			break;
		}
		//���ցB
		m_currentKeyFrameNo++;
	}

	if (!isPlayAnimEvent) { return; }

	//�{�[���s����v�Z���Ă����B
	const auto& keyFramePtrListArray = m_animationClip->GetKeyFramePtrListArray();
	for (const auto& keyFrameList : keyFramePtrListArray) {
		if (keyFrameList.size() == 0) {
			continue;
		}
		//���ݍĐ����̃L�[�t���[��������Ă���B
		Keyframe* keyframe = keyFrameList.at(m_currentKeyFrameNo);
		if (keyframe->boneIndex < m_boneMatrix.size()) {
			m_boneMatrix[keyframe->boneIndex] = keyframe->transform;

			//�O�̃L�[�t���[���Ƃ̕⊮
			if (m_currentKeyFrameNo > 0 && m_time < keyframe->time) {
				Keyframe* prevKeyframe = keyFrameList.at(m_currentKeyFrameNo - 1);
				CMatrix prevKeyFrameBoneMatrix;
				prevKeyFrameBoneMatrix = prevKeyframe->transform;

				//�u�����h
				float blend = (m_time - prevKeyframe->time) / (keyframe->time - prevKeyframe->time);

				CMatrix& m = m_boneMatrix[keyframe->boneIndex];
				//���s�ړ��̕⊮
				CVector3 move;
				move.Lerp(
					blend,
					*(CVector3*)prevKeyFrameBoneMatrix.m[3],
					*(CVector3*)m.m[3]
				);
				//���s�ړ��������폜�B
				m.m[3][0] = 0.0f;
				m.m[3][1] = 0.0f;
				m.m[3][2] = 0.0f;
				prevKeyFrameBoneMatrix.m[3][0] = 0.0f;
				prevKeyFrameBoneMatrix.m[3][1] = 0.0f;
				prevKeyFrameBoneMatrix.m[3][2] = 0.0f;

				//�g�听���̕�ԁB
				CVector3 vBoneScale, vBoneScalePrev;
				vBoneScale.x = (*(CVector3*)m.m[0]).Length();
				vBoneScale.y = (*(CVector3*)m.m[1]).Length();
				vBoneScale.z = (*(CVector3*)m.m[2]).Length();
				vBoneScalePrev.x = (*(CVector3*)prevKeyFrameBoneMatrix.m[0]).Length();
				vBoneScalePrev.y = (*(CVector3*)prevKeyFrameBoneMatrix.m[1]).Length();
				vBoneScalePrev.z = (*(CVector3*)prevKeyFrameBoneMatrix.m[2]).Length();
				CVector3 scale;
				scale.Lerp(
					blend,
					vBoneScalePrev,
					vBoneScale
				);
				//�g�听���������B
				m.m[0][0] /= vBoneScale.x;
				m.m[0][1] /= vBoneScale.x;
				m.m[0][2] /= vBoneScale.x;
				m.m[1][0] /= vBoneScale.y;
				m.m[1][1] /= vBoneScale.y;
				m.m[1][2] /= vBoneScale.y;
				m.m[2][0] /= vBoneScale.z;
				m.m[2][1] /= vBoneScale.z;
				m.m[2][2] /= vBoneScale.z;
				prevKeyFrameBoneMatrix.m[0][0] /= vBoneScalePrev.x;
				prevKeyFrameBoneMatrix.m[0][1] /= vBoneScalePrev.x;
				prevKeyFrameBoneMatrix.m[0][2] /= vBoneScalePrev.x;
				prevKeyFrameBoneMatrix.m[1][0] /= vBoneScalePrev.y;
				prevKeyFrameBoneMatrix.m[1][1] /= vBoneScalePrev.y;
				prevKeyFrameBoneMatrix.m[1][2] /= vBoneScalePrev.y;
				prevKeyFrameBoneMatrix.m[2][0] /= vBoneScalePrev.z;
				prevKeyFrameBoneMatrix.m[2][1] /= vBoneScalePrev.z;
				prevKeyFrameBoneMatrix.m[2][2] /= vBoneScalePrev.z;

				//��]�̕⊮
				CQuaternion qBone, qBonePrev;
				qBone.SetRotation(m);
				qBonePrev.SetRotation(prevKeyFrameBoneMatrix);
				CQuaternion rot;
				rot.Slerp(blend, qBonePrev, qBone);

				//�g��s����쐬�B
				CMatrix scaleMatrix;
				scaleMatrix.MakeScaling(scale);
				//��]�s����쐬�B
				CMatrix rotMatrix;
				rotMatrix.MakeRotationFromQuaternion(rot);
				//�S�����������āA�{�[���s����쐬�B
				m_boneMatrix[keyframe->boneIndex].Mul(scaleMatrix, rotMatrix);
				m_boneMatrix[keyframe->boneIndex].SetTranslation(move);
			}
		}
#ifndef DW_MASTER	
		else {		
			MessageBox(NULL, "AnimationPlayController::Update : ���݂��Ȃ��{�[���ɒl�������������Ƃ��Ă��܂��B���̂悤�Ȍ������l�����܂��B\n"
				"�@ tka�t�@�C�����o�͂��鎞�ɁA�I���������[�g�{�[�����X�P���g���̃��[�g�{�[���ƈقȂ��Ă���B\n"
				"�A �قȂ�X�P���g���̃A�j���[�V�����N���b�v���g���Ă���B\n"
				"������xtka�t�@�C�����o�͂��Ȃ����Ă��������B", "error", MB_OK);
			std::abort();
		}
#endif
	}
}

}