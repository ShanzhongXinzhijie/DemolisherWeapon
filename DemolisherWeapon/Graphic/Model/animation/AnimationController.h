#pragma once
#include "Animation.h"

namespace DemolisherWeapon {

class AnimationController
{
public:
	AnimationController();
	~AnimationController();

	//初期化
	void Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip);

	//アニメーションを追加
	//戻り値　:何番目か
	int AddAnimation();

	//アニメーションを取得
	Animation& GetAnimation(int index) {
		return m_animations[index];
	}

	//アニメーションが再生中か調べる
	bool IsPlaying() const {
		return m_animations[0].IsPlaying();
	}

	//アニメーションを再生
	void Play(int clipNo, float interpolateTimeSec = 0.0f, bool replay = false) {
		m_animations[0].Play(clipNo, interpolateTimeSec, replay);
	}
	void Replay(int clipNo, float interpolateTimeSec = 0.0f) {
		Play(clipNo, interpolateTimeSec, true);
	}

	//アニメーションの再生速度を設定
	void SetSpeed(float speedScale) {
		m_animPlaySpeedSec[0] = speedScale;
	}
	void SetSpeed(int index,float speedScale) {
		m_animPlaySpeedSec[index] = speedScale;
	}

	/*!
	*@brief	アニメーションイベントを監視する関数を登録。
	*@details
	* アニメーションイベントが打ち込まれたフレームまでアニメーションを
	* 再生すると、登録されている関数が呼び出されます。
	*@param[in]		eventListener		イベントを監視する関数オブジェクト。
	*/
	void AddAnimationEventListener(std::function<void(const wchar_t* clipName, const wchar_t* eventName)> eventListener)
	{
		m_animations[0].AddAnimationEventListener(eventListener);
	}

	void Update();
	void Update(float updateTime);//指定時間アニメーション進める

private:

	AnimationClip*	m_animationClips = nullptr;
	int m_numAnimClip = 0;
	SkinModel* m_skinModel = nullptr;
	Skeleton* m_skeleton = nullptr;

	std::vector<Animation> m_animations;
	std::vector<float> m_animPlaySpeedSec;
};

}