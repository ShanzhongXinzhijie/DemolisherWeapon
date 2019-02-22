#pragma once
#include "Animation.h"

namespace DemolisherWeapon {

class AnimationController
{
public:
	AnimationController();
	~AnimationController();

	//������
	void Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip);

	//�A�j���[�V������ǉ�
	//�߂�l�@:���Ԗڂ�
	int AddAnimation();

	//�A�j���[�V�������擾
	Animation& GetAnimation(int index) {
		return m_animations[index];
	}

	//�A�j���[�V�������Đ��������ׂ�
	bool IsPlaying() const {
		return m_animations[0].IsPlaying();
	}

	//�A�j���[�V�������Đ�
	void Play(int clipNo, float interpolateTime = 0.0f, bool replay = false) {
		m_animations[0].Play(clipNo, interpolateTime, replay);
	}
	void Replay(int clipNo, float interpolateTime = 0.0f) {
		Play(clipNo, interpolateTime, true);
	}

	//�A�j���[�V�����̍Đ����x��ݒ�
	void SetSpeed(float speedScale) {
		m_animPlaySpeedSec[0] = speedScale;
	}
	void SetSpeed(int index,float speedScale) {
		m_animPlaySpeedSec[index] = speedScale;
	}

	/*!
	*@brief	�A�j���[�V�����C�x���g���Ď�����֐���o�^�B
	*@details
	* �A�j���[�V�����C�x���g���ł����܂ꂽ�t���[���܂ŃA�j���[�V������
	* �Đ�����ƁA�o�^����Ă���֐����Ăяo����܂��B
	*@param[in]		eventListener		�C�x���g���Ď�����֐��I�u�W�F�N�g�B
	*/
	void AddAnimationEventListener(std::function<void(const wchar_t* clipName, const wchar_t* eventName)> eventListener)
	{
		m_animations[0].AddAnimationEventListener(eventListener);
	}

	void Update();
	void Update(float updateTime);//�w�莞�ԃA�j���[�V�����i�߂�

private:

	AnimationClip*	m_animationClips = nullptr;
	int m_numAnimClip = 0;
	SkinModel* m_skinModel = nullptr;
	Skeleton* m_skeleton = nullptr;

	std::vector<Animation> m_animations;
	std::vector<float> m_animPlaySpeedSec;
};

}