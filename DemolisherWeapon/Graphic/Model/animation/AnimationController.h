#pragma once
#include "Animation.h"

namespace DemolisherWeapon {

class AnimationController
{
public:
	AnimationController();
	~AnimationController();

	void Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip);

	//アニメーションを追加
	//戻り値　:何番目か
	int AddAnimation();

	//アニメーションを取得
	Animation& GetAnimation(int index) {
		return m_animations[index];
	}

	//アニメーションを再生
	void Play(int clipNo, float interpolateTime = 0.0f) {
		m_animations[0].Play(clipNo, interpolateTime);
	}

	//アニメーションの再生速度を設定
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