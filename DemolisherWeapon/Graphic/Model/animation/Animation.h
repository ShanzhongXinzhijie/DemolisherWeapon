/*!
 * @brief	アニメーション
 */

#pragma once

#include "AnimationClip.h"
#include "AnimationPlayController.h"

namespace DemolisherWeapon {

class Skeleton;
class SkinModel;

using AnimationEventListener = std::function<void(const wchar_t* clipName, const wchar_t* eventName)>;

/*!
* @brief	アニメーションクラス。
*/
class Animation {
public:
	Animation();
	~Animation();

	/*!
	*@brief	初期化。
	*@param[in]	skinModel		アニメーションさせるスキンモデル。
	*@param[in]	animeClipList	アニメーションクリップの配列。
	*@param[in]	numAnimClip		アニメーションクリップの数。
	*/
	void Init(SkinModel& skinModel, AnimationClip animClipList[], int numAnimClip);
		
	/*!
	*@brief	アニメーションの再生。
	*@param[in]	clipNo	アニメーションクリップの番号。Init関数に渡したanimClipListの並びとなる。
	*@param[in]	interpolateTime		補完時間(単位：秒)
	*/
	void Play(int clipNo, float interpolateTime = 0.0f)
	{
		PlayCommon(m_animationClips[clipNo], interpolateTime);
	}
	/*!
	* @brief	アニメーションの再生中？
	*/
	bool IsPlaying() const
	{
		int lastIndex = GetLastAnimationControllerIndex();
		return m_animationPlayControllerPtr[lastIndex]->IsPlaying();
	}

	/*!
	*@brief	アニメーションイベントリスナーを登録。
	*@return
	* 登録されたリスナー。
	*/
	void AddAnimationEventListener(AnimationEventListener eventListener)
	{
		m_animationEventListeners.push_back(eventListener);
	}

	/*!
	* @brief	アニメーションイベントをリスナーに通知。
	*/
	void NotifyAnimationEventToListener(const wchar_t* clipName, const wchar_t* eventName)
	{
		for (auto& listener : m_animationEventListeners) {
			listener(clipName, eventName);
		}
	}
	
	/*!
	* @brief	アニメーションを進める。
	*@details
	* エンジン内部から呼ばれます。
	* ユーザーは使用しないでください。
	*@param[in]	deltaTime		アニメーションを進める時間(単位：秒)。
	*/
	void Update(float deltaTime);

	//ボーンの状態を取得
	void GetGlobalPose(int index, CVector3& trans, CQuaternion& rot, CVector3& scale) {
		rot = m_GlobalRotation[index];
		trans = m_GlobalTranslation[index];
		scale = m_GlobalScale[index];
	}
	
private:
	void PlayCommon(AnimationClip* nextClip, float interpolateTime)
	{
		//最終ポーズのインデックス取得
		int index = GetLastAnimationControllerIndex();
		//最終ポーズと同じアニメーションならreturn
		if (m_animationPlayControllerPtr[index]->GetAnimClip() == nextClip) {
			return;
		}

		if (interpolateTime == 0.0f) {
			//補完なし。
			m_numAnimationPlayController = 1;
			index = GetLastAnimationControllerIndex();
		}
		else {
			if (m_numAnimationPlayController >= ANIMATION_PLAY_CONTROLLER_NUM) {
				//アニメーションの数が上限を迎えていたら(最大数より増えるべきでない(2度参照されるべきではない))

				//一番補完率の低いものを上書き
				index = GetMinInterpolateRateAnimationControllerIndex();
				//それ以外の位置を前にずらして、一番うしろに持ってくる
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
				//補完あり。
				m_numAnimationPlayController++;
				index = GetLastAnimationControllerIndex();
			}
		}

		//再生
		m_animationPlayControllerPtr[index]->ChangeAnimationClip(nextClip);
		m_animationPlayControllerPtr[index]->SetInterpolateTime(interpolateTime);

		//補完率のリセット
		m_interpolateTimeEnd = interpolateTime;
		m_interpolateTime = 0.0f;
	}
	/*!
	* @brief	ローカルポーズの更新。
	*/
	void UpdateLocalPose(float deltaTime);
	/*!
	* @brief	グローバルポーズの更新。
	*/
	void UpdateGlobalPose();
		
private:
		
	//補完率が一番低いアニメーションコントローラーのインデックスを取得
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
		*@brief	最終ポーズになるアニメーションのリングバッファ上でのインデックスを取得。
	*/
	int GetLastAnimationControllerIndex() const
	{
		return GetAnimationControllerIndex(m_startAnimationPlayController, m_numAnimationPlayController - 1);
	}
	/*!
	*@brief	アニメーションコントローラのリングバッファ上でのインデックスを取得。
	*@param[in]	startIndex		開始インデックス。
	*@param[in]	localIndex		ローカルインデックス。
	*/
	int GetAnimationControllerIndex(int startIndex, int localIndex) const
	{
		return (startIndex + localIndex) % ANIMATION_PLAY_CONTROLLER_NUM;
	}
private:
	static const int ANIMATION_PLAY_CONTROLLER_NUM = 3;			//!<アニメーションコントローラの数。
	std::vector<AnimationClip*>	m_animationClips;					//!<アニメーションクリップの配列。
	Skeleton* m_skeleton = nullptr;	//!<アニメーションを適用するスケルトン。
	AnimationPlayController* m_animationPlayControllerPtr[ANIMATION_PLAY_CONTROLLER_NUM];	//!<アニメーションコントローラ。リングバッファ。
	AnimationPlayController  m_animationPlayControllerSource[ANIMATION_PLAY_CONTROLLER_NUM]; 
	int m_numAnimationPlayController = 0;		//!<現在使用中のアニメーション再生コントローラの数。
	int m_startAnimationPlayController = 0;		//!<アニメーションコントローラの開始インデックス。
	float m_interpolateTime = 0.0f;
	float m_interpolateTimeEnd = 0.0f;
	bool m_isInterpolate = false;				//!<補間中？

	//出力するグローバルポーズ
	std::vector<CQuaternion> m_GlobalRotation;
	std::vector<CVector3> m_GlobalTranslation;
	std::vector<CVector3> m_GlobalScale;

	std::vector<AnimationEventListener>	m_animationEventListeners;	//!<アニメーションイベントリスナーのリスト。

};

}