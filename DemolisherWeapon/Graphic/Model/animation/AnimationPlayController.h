/*!
 * @brief	アニメーション再生コントローラ。
 */

#pragma once

namespace DemolisherWeapon {

class Skeleton;
class AnimationClip;
class Bone;
class Animation;

/*!
* @brief	アニメーションの再生コントローラ。
* @details
*  一つのアニメーションクリップに対してアニメーションを進めて、ローカルポーズを計算します。
*/
class AnimationPlayController{
public:
	/*!
	* @brief	コンストラクタ。
	*/
	AnimationPlayController()
	{
	}
	/*!
	* @brief	デストラクタ。
	*/
	~AnimationPlayController()
	{
	}
	/*!
	*@brief	初期化。
	*/
	void Init(Skeleton* skeleton);
	/*!
	* @brief	アニメーションクリップの変更。
	*/
	void ChangeAnimationClip(AnimationClip* clip)
	{
		m_animationClip = clip;
		m_currentKeyFrameNo = 0;
		m_time = 0.0f;
		m_isPlaying = true;
	}
	void SetInterpolateTime(float interpolateTime)
	{
		if (interpolateTime < 0.0f) {
			interpolateTime = 0.0f;
		}
		m_interpolateEndTime = interpolateTime;
		m_interpolateTime = 0.0f;
	}
	/*!
	*@brief	補完率を取得。
	*/
	float GetInterpolateRate() const
	{
		if (m_interpolateEndTime <= 0.0f) {
			return 1.0f;
		}
		return min( 1.0f, m_interpolateTime / m_interpolateEndTime );
	}
	/*!
	* @brief	アニメーションを進める。
	*@param[in]	deltaTime		アニメーションを進める時間。
	*/
	void Update(float deltaTime, Animation* animation);
	/*!
	* @brief	ローカルボーン行列を取得。
	*/
	const std::vector<CMatrix>& GetBoneLocalMatrix() const
	{
		return m_boneMatrix;
	}
	AnimationClip* GetAnimClip() const
	{
		return m_animationClip;
	}
	/*!
	* @brief	再生中？
	*/
	bool IsPlaying() const
	{
		return m_isPlaying;
	}
private:
	
	/*!
	*@brief	アニメーションイベントを起動する。
	*/
	void InvokeAnimationEvent(Animation* animation);

	/*!
	*@brief	ループ再生開始する時の処理。
	*/
	void StartLoop();

private:
	AnimationClip*			m_animationClip = nullptr;		//!<アニメーションクリップ。
	int						m_currentKeyFrameNo = 0;		//!<現在再生中のキーフレーム番号。
	float					m_time = 0.0f;
	std::vector<CMatrix>	m_boneMatrix;				//!<このコントローラで再生中のアニメーションのボーン行列。
	float					m_interpolateTime = 0.0f;			//!<補完時間
	float					m_interpolateEndTime = 0.0f;		//!<補完終了時間
	bool					m_isPlaying = false;		//!<再生中？
};

}