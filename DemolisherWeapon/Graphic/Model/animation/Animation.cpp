/*!
 * @brief	�A�j���[�^�[�N���X�B
 */


#include "DWstdafx.h"
#include "Animation.h"
#include "Graphic/Model/skeleton.h"
#include "Graphic/Model/skinModel.h"

namespace DemolisherWeapon {

Animation::Animation()
{
}
Animation::~Animation()
{
	
}
	
void Animation::Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip)
{
	if (animClipList == nullptr) {
#ifndef DW_MASTER
		char message[256];
		strcpy_s(message, "animClipList��NULL�ł��B\n");
		OutputDebugStringA(message);
		//�~�߂�B
		std::abort();
#endif
		
	}
	m_skeleton = &skinModel.GetSkeleton();

	for (int i = 0; i < numAnimClip; i++) {
		m_animationClips.push_back(&animClipList[i]);
	}
	for (auto& ctr : m_animationPlayControllerSource) {
		ctr.Init(m_skeleton);
	}
	for (int i = 0; i < ANIMATION_PLAY_CONTROLLER_NUM; i++) {
		m_animationPlayControllerPtr[i] = &m_animationPlayControllerSource[i];
	}

	//�m��
	int numBones = m_skeleton->GetNumBones();
	m_GlobalRotation.resize(numBones);
	m_GlobalTranslation.resize(numBones);
	m_GlobalScale.resize(numBones);
		
	Play(0);
}
/*!
* @brief	���[�J���|�[�Y�̍X�V�B
*/
void Animation::UpdateLocalPose(float deltaTime)
{
	m_interpolateTime += deltaTime;
	if (m_interpolateTime >= m_interpolateTimeEnd) {
		//��Ԋ����B
		//���݂̍ŏI�A�j���[�V�����R���g���[���ւ̃C���f�b�N�X���J�n�C���f�b�N�X�ɂȂ�B
		m_startAnimationPlayController = GetLastAnimationControllerIndex();
		m_numAnimationPlayController = 1;
		m_interpolateTime = m_interpolateTimeEnd;
	}
	//AnimationPlayController::Update�֐������s���Ă����B
	for (int i = 0; i < m_numAnimationPlayController; i++) {
		int index = GetAnimationControllerIndex(m_startAnimationPlayController, i );
		m_animationPlayControllerPtr[index]->Update(deltaTime, this);
	}
}

void Animation::UpdateGlobalPose()
{
	//������
	int numBone = m_skeleton->GetNumBones();
	for (int i = 0; i < numBone; i++) {
		m_GlobalRotation[i] = CQuaternion::Identity();
		m_GlobalTranslation[i] = CVector3::Zero();
		m_GlobalScale[i] = CVector3::One();
	}

	//�O���[�o���|�[�Y���v�Z���Ă����B
	int startIndex = m_startAnimationPlayController;
	for (int i = 0; i < m_numAnimationPlayController; i++) {
		int index = GetAnimationControllerIndex(startIndex, i);
		float intepolateRate = m_animationPlayControllerPtr[index]->GetInterpolateRate();
		const auto& localBoneMatrix = m_animationPlayControllerPtr[index]->GetBoneLocalMatrix();
		for (int boneNo = 0; boneNo < numBone; boneNo++) {
			//���s�ړ��̕⊮
			CMatrix m = localBoneMatrix[boneNo];
			m_GlobalTranslation[boneNo].Lerp(
				intepolateRate, 
				m_GlobalTranslation[boneNo],
				*(CVector3*)m.m[3]
			);
			//���s�ړ��������폜�B
			m.m[3][0] = 0.0f;
			m.m[3][1] = 0.0f;
			m.m[3][2] = 0.0f;
			
			//�g�听���̕�ԁB
			CVector3 vBoneScale;
			vBoneScale.x = (*(CVector3*)m.m[0]).Length();
			vBoneScale.y = (*(CVector3*)m.m[1]).Length();
			vBoneScale.z = (*(CVector3*)m.m[2]).Length();

			m_GlobalScale[boneNo].Lerp(
				intepolateRate,
				m_GlobalScale[boneNo],
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
				
			//��]�̕⊮
			CQuaternion qBone;
			qBone.SetRotation(m);
			m_GlobalRotation[boneNo].Slerp(intepolateRate, m_GlobalRotation[boneNo], qBone);
		}
	}
		
	//�ŏI�A�j���[�V�����ȊO�͕�Ԋ������Ă����珜�����Ă����B
	int numAnimationPlayController = m_numAnimationPlayController;
	for (int i = 1; i < m_numAnimationPlayController; i++) {
		int index = GetAnimationControllerIndex(startIndex, i);
		if (m_animationPlayControllerPtr[index]->GetInterpolateRate() > 1.0f-FLT_EPSILON) {
			//��Ԃ��I����Ă���̂ŃA�j���[�V�����̊J�n�ʒu��O�ɂ���B
			m_startAnimationPlayController = index;
			numAnimationPlayController = m_numAnimationPlayController - i;
		}
	}
	m_numAnimationPlayController = numAnimationPlayController;
}
	

	
void Animation::Update(float deltaTime)
{
	if (m_numAnimationPlayController == 0) {
		return;
	}
	//���[�J���|�[�Y�̍X�V������Ă����B
	UpdateLocalPose(deltaTime);
		
	//�O���[�o���|�[�Y���v�Z���Ă����B
	UpdateGlobalPose();
}

}