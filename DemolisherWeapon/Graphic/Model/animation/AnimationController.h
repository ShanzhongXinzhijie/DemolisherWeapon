#pragma once
#include "Animation.h"

namespace DemolisherWeapon {

class AnimationController
{
public:
	AnimationController();
	~AnimationController();

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
	void Play(int clipNo, float interpolateTime = 0.0f) {
		m_animations[0].Play(clipNo, interpolateTime);
	}

	//�A�j���[�V�����̍Đ����x��ݒ�
	void SetSpeed(float speedScale) {
		m_animPlaySpeedSec[0] = speedScale;
	}
	void SetSpeed(int index,float speedScale) {
		m_animPlaySpeedSec[index] = speedScale;
	}

	void Update();

private:

	AnimationClip*	m_animationClips;
	int m_numAnimClip = 0;
	SkinModel* m_skinModel = nullptr;
	Skeleton* m_skeleton = nullptr;

	std::vector<Animation> m_animations;
	std::vector<float> m_animPlaySpeedSec;
};

}