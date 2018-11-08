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
	
void AnimationPlayController::InvokeAnimationEvent(Animation* animation)
{
	auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		if (m_time > animEventArray[i].GetInvokeTime()
			&& animEventArray[i].IsInvoked() == false) {
			//�A�j���[�V�����̋N�����Ԃ��߂��Ă��銎�A�܂��C�x���g�N�����Ă��Ȃ��B
			animation->NotifyAnimationEventToListener(
				m_animationClip->GetName(), animEventArray[i].GetEventName()
			);
			animEventArray[i].SetInvokedFlag(true);
		}
	}
}
	
void AnimationPlayController::StartLoop()
{
	m_currentKeyFrameNo = 0;
	m_time = 0.0f;
}
void AnimationPlayController::Update(float deltaTime, Animation* animation)
{
	if(m_animationClip == nullptr){
		
		return ;
	}
	const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();
	m_time += deltaTime;

	//�⊮���Ԃ��i�߂Ă����B
	m_interpolateTime = min(m_interpolateEndTime, m_interpolateTime + deltaTime);
	while (true) {
		if (m_currentKeyFrameNo >= (int)topBoneKeyFrameList.size()) {
			//�I�[�܂ōs�����B
			if (m_animationClip->IsLoop()) {
				//���[�v�B
				StartLoop();
			}
			else {
				//�����V���b�g�Đ��B
				m_currentKeyFrameNo--;
				m_isPlaying = false;	//�Đ��I���B
			}
			break;
		}
		if (topBoneKeyFrameList.at(m_currentKeyFrameNo)->time >= m_time) {
			//�I���B
			break;
		}
		//���ցB
		m_currentKeyFrameNo++;
	}
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
		}
		else {
#ifdef _DEBUG			
			MessageBox(NULL, "AnimationPlayController::Update : ���݂��Ȃ��{�[���ɒl�������������Ƃ��Ă��܂��B���̂悤�Ȍ������l�����܂��B\n"
				"�@ tka�t�@�C�����o�͂��鎞�ɁA�I���������[�g�{�[�����X�P���g���̃��[�g�{�[���ƈقȂ��Ă���B\n"
				"�A �قȂ�X�P���g���̃A�j���[�V�����N���b�v���g���Ă���B\n"
				"������xtka�t�@�C�����o�͂��Ȃ����Ă��������B", "error", MB_OK);
			std::abort();
#endif
		}
	}
}

}