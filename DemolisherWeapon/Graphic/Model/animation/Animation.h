/*!
 * @brief	�A�j���[�V����
 */

#pragma once

#include "AnimationClip.h"
#include "AnimationPlayController.h"

namespace DemolisherWeapon {

class Skeleton;
class SkinModel;

using AnimationEventListener = std::function<void(const wchar_t* clipName, const wchar_t* eventName)>;

/*!
* @brief	�A�j���[�V�����N���X�B
*/
class Animation {
public:
	Animation();
	~Animation();

	/*!
	*@brief	�������B
	*@param[in]	skinModel		�A�j���[�V����������X�L�����f���B
	*@param[in]	animeClipList	�A�j���[�V�����N���b�v�̔z��B
	*@param[in]	numAnimClip		�A�j���[�V�����N���b�v�̐��B
	*/
	void Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip);
		
	/*!
	*@brief	�A�j���[�V�����̍Đ��B
	*@param[in]	clipNo	�A�j���[�V�����N���b�v�̔ԍ��BInit�֐��ɓn����animClipList�̕��тƂȂ�B
	*@param[in]	interpolateTime		�⊮����(�P�ʁF�b)
	*/
	void Play(int clipNo, float interpolateTime = 0.0f, bool replay = false)
	{
		PlayCommon(m_animationClips[clipNo], interpolateTime, replay);
	}
	/*!
	* @brief	�A�j���[�V�����̍Đ����H
	*/
	bool IsPlaying() const
	{
		int lastIndex = GetLastAnimationControllerIndex();
		return m_animationPlayControllerPtr[lastIndex]->IsPlaying();
	}

	/*!
	*@brief	�A�j���[�V�����C�x���g���X�i�[��o�^�B
	*@return
	* �o�^���ꂽ���X�i�[�B
	*/
	void AddAnimationEventListener(AnimationEventListener eventListener)
	{
		m_animationEventListeners.push_back(eventListener);
	}

	/*!
	* @brief	�A�j���[�V�����C�x���g�����X�i�[�ɒʒm�B
	*/
	void NotifyAnimationEventToListener(const wchar_t* clipName, const wchar_t* eventName)
	{
		for (auto& listener : m_animationEventListeners) {
			listener(clipName, eventName);
		}
	}
	
	/*!
	* @brief	�A�j���[�V������i�߂�B
	*@details
	* �G���W����������Ă΂�܂��B
	* ���[�U�[�͎g�p���Ȃ��ł��������B
	*@param[in]	deltaTime		�A�j���[�V������i�߂鎞��(�P�ʁF�b)�B
	*/
	void Update(float deltaTime);

	//�{�[���̏�Ԃ��擾
	void GetGlobalPose(int index, CVector3& trans, CQuaternion& rot, CVector3& scale) {
		rot = m_GlobalRotation[index];
		trans = m_GlobalTranslation[index];
		scale = m_GlobalScale[index];
	}
	
private:
	void PlayCommon(AnimationClip* nextClip, float interpolateTime, bool replay)
	{
		//�ŏI�|�[�Y�̃C���f�b�N�X�擾
		int index = GetLastAnimationControllerIndex();
		//replay==false�ōŏI�|�[�Y�Ɠ����A�j���[�V�����Ȃ�return
		if (m_animationPlayControllerPtr[index]->GetAnimClip() == nextClip && !replay) {
			return;
		}

		if (interpolateTime == 0.0f) {
			//�⊮�Ȃ��B
			m_numAnimationPlayController = 1;
			index = GetLastAnimationControllerIndex();
		}
		else {
			if (m_numAnimationPlayController >= ANIMATION_PLAY_CONTROLLER_NUM) {
				//�A�j���[�V�����̐���������}���Ă�����(�ő吔��葝����ׂ��łȂ�(2�x�Q�Ƃ����ׂ��ł͂Ȃ�))

				//��ԕ⊮���̒Ⴂ���̂��㏑��
				index = GetMinInterpolateRateAnimationControllerIndex();
				//����ȊO�̈ʒu��O�ɂ��炵�āA��Ԃ�����Ɏ����Ă���
				AnimationPlayController* lastptr = m_animationPlayControllerPtr[index];
				bool enable = false; int previndex = 0;
				for (int i = 0; i < m_numAnimationPlayController; i++) {
					int l_index = GetAnimationControllerIndex(m_startAnimationPlayController, i);
					if (enable) {
						m_animationPlayControllerPtr[previndex] = m_animationPlayControllerPtr[l_index];
						if (i == m_numAnimationPlayController - 1) {
							m_animationPlayControllerPtr[l_index] = lastptr;
							index = l_index;
							break;
						}
					}
					if (l_index == index) {
						enable = true;
					}
					previndex = l_index;
				}
			}
			else {
				//�⊮����B
				m_numAnimationPlayController++;
				index = GetLastAnimationControllerIndex();
			}
		}

		//�Đ�
		m_animationPlayControllerPtr[index]->ChangeAnimationClip(nextClip);
		if (abs(nextClip->GetStartTimeOffset()) > 0.0f) {
			//�I�t�Z�b�g���i�߂�
			m_animationPlayControllerPtr[index]->Update(nextClip->GetStartTimeOffset(), this, false);
		}
		m_animationPlayControllerPtr[index]->SetInterpolateTime(interpolateTime);

		//�⊮���̃��Z�b�g
		m_interpolateTimeEnd = interpolateTime;
		m_interpolateTime = 0.0f;
	}
	/*!
	* @brief	���[�J���|�[�Y�̍X�V�B
	*/
	void UpdateLocalPose(float deltaTime);
	/*!
	* @brief	�O���[�o���|�[�Y�̍X�V�B
	*/
	void UpdateGlobalPose();
		
private:
		
	//�⊮������ԒႢ�A�j���[�V�����R���g���[���[�̃C���f�b�N�X���擾
	int GetMinInterpolateRateAnimationControllerIndex()const {
		int index = 0; float minInterpolateRate = 2.0f;
		for (int i = 0; i < m_numAnimationPlayController; i++) {
			int l_index = GetAnimationControllerIndex(m_startAnimationPlayController, i);

			float rate = m_animationPlayControllerPtr[l_index]->GetInterpolateRate();
			if (rate <= minInterpolateRate) {
				minInterpolateRate = rate;
				index = l_index;
			}
		}
		return index;
	}

	/*!
		*@brief	�ŏI�|�[�Y�ɂȂ�A�j���[�V�����̃����O�o�b�t�@��ł̃C���f�b�N�X���擾�B
	*/
	int GetLastAnimationControllerIndex() const
	{
		return GetAnimationControllerIndex(m_startAnimationPlayController, m_numAnimationPlayController - 1);
	}
	/*!
	*@brief	�A�j���[�V�����R���g���[���̃����O�o�b�t�@��ł̃C���f�b�N�X���擾�B
	*@param[in]	startIndex		�J�n�C���f�b�N�X�B
	*@param[in]	localIndex		���[�J���C���f�b�N�X�B
	*/
	int GetAnimationControllerIndex(int startIndex, int localIndex) const
	{
		return (startIndex + localIndex) % ANIMATION_PLAY_CONTROLLER_NUM;
	}
private:
	static const int ANIMATION_PLAY_CONTROLLER_NUM = 3;			//!<�A�j���[�V�����R���g���[���̐��B
	std::vector<AnimationClip*>	m_animationClips;					//!<�A�j���[�V�����N���b�v�̔z��B
	Skeleton* m_skeleton = nullptr;	//!<�A�j���[�V������K�p����X�P���g���B
	AnimationPlayController* m_animationPlayControllerPtr[ANIMATION_PLAY_CONTROLLER_NUM];	//!<�A�j���[�V�����R���g���[���B�����O�o�b�t�@�B
	AnimationPlayController  m_animationPlayControllerSource[ANIMATION_PLAY_CONTROLLER_NUM]; 
	int m_numAnimationPlayController = 0;		//!<���ݎg�p���̃A�j���[�V�����Đ��R���g���[���̐��B
	int m_startAnimationPlayController = 0;		//!<�A�j���[�V�����R���g���[���̊J�n�C���f�b�N�X�B
	float m_interpolateTime = 0.0f;
	float m_interpolateTimeEnd = 0.0f;
	bool m_isInterpolate = false;				//!<��Ԓ��H

	//�o�͂���O���[�o���|�[�Y
	std::vector<CQuaternion> m_GlobalRotation;
	std::vector<CVector3> m_GlobalTranslation;
	std::vector<CVector3> m_GlobalScale;

	std::vector<AnimationEventListener>	m_animationEventListeners;	//!<�A�j���[�V�����C�x���g���X�i�[�̃��X�g�B

};

}