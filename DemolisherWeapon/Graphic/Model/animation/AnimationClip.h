/*!
 *@brief	アニメーションクリップ。
 */

#pragma once

namespace DemolisherWeapon {

//////////////////////////////////////////////////////////////////////
// 各種構造体
//////////////////////////////////////////////////////////////////////
/*!
*@brief	アニメーションクリップのヘッダー。
*/
struct AnimClipHeader {
	std::uint32_t numKey;				//!<キーフレームの数。
	std::uint32_t numAnimationEvent;	//!<アニメーションイベントの数。
};
/*!
*@brief	アニメーションイベント
*@アニメーションイベントは未対応。
* やりたかったら自分で実装するように。
*/
struct AnimationEventData {
	float	invokeTime;					//!<アニメーションイベントが発生する時間(単位:秒)
	std::uint32_t eventNameLength;		//!<イベント名の長さ。
};
/*!
*@brief	キーフレーム。
*/
struct Keyframe {
	std::uint32_t boneIndex;	//!<ボーンインデックス。
	float time;					//!<時間。
	CMatrix transform;			//!<トランスフォーム。
};
/*!
*@brief	キーフレーム。
*/
struct KeyframeRow {
	std::uint32_t boneIndex;	//!<ボーンインデックス。
	float time;					//!<時間。
	CVector3 transform[4];		//!<トランスフォーム。
};

//軸変換の種類
enum EnChangeAnimationClipUpAxis {
	enNonChange,
	enZtoY,
	enYtoZ,
};

/*!
	*@brief	アニメーションイベント。
	*/
class AnimationEvent {
public:
	AnimationEvent()
	{
	}
	/*!
	*@brief	イベント発生時間を設定。
	*/
	float GetInvokeTime() const
	{
		return m_invokeTime;
	}
	/*!
	*@brief	イベント名を取得。
	*/
	const wchar_t* GetEventName() const
	{
		return m_eventName.c_str();
	}
	/*!
	*@brief	イベント発生時間を設定。
	*/
	void SetInvokeTime(float time)
	{
		m_invokeTime = time;
	}
	/*!
	*@brief	イベント名を設定。
	*/
	void SetEventName(const wchar_t* name)
	{
		m_eventName = name;
	}
	/*!
	*@brief	イベントが発生済みか判定。
	*/
	bool IsInvoked() const
	{
		return m_isInvoked;
	}
	/*!
	*@brief	イベントが発生済みのフラグを設定する。
	*/
	void SetInvokedFlag(bool flag)
	{
		m_isInvoked = flag;
	}
private:
	bool m_isInvoked = false;	//!<イベント発生済み？
	float m_invokeTime;			//!<イベント発生時間。
	std::wstring m_eventName;	//!<イベント名。
};

/*!
*@brief	アニメーションクリップ。
*/
class AnimationClip  {
public:
	//タイプ量が長ったらしくて、うざいのでstd::vector<KeyFrame*>の別名定義。
	using keyFramePtrList = std::vector<Keyframe*>;
	/*!
	* @brief	コンストラクタ
	*/
	AnimationClip() 
	{
	}
	/*!
	*@brief	デストラクタ。
	*/
	~AnimationClip();
	/*!
	*@brief	アニメーションクリップをロード。
	*@param[in]	filePath	ファイルパス。
	*/
	void Load(const wchar_t* filePath, bool loop = false, EnChangeAnimationClipUpAxis changeUpAxis = enNonChange);
	void Load(const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis) {
		Load(filePath, false, changeUpAxis);
	};

	/*!
	*@brief	ループする？
	*/
	bool IsLoop() const
	{
		return m_isLoop;
	}
	/*!
	*@brief	ループフラグを設定する。
	*/
	void SetLoopFlag(bool flag)
	{
		m_isLoop = flag;
	}
	/*!
	*@brief
	*/
	const std::vector<keyFramePtrList>& GetKeyFramePtrListArray() const
	{
		return m_keyFramePtrListArray;
	}
	const keyFramePtrList& GetTopBoneKeyFrameList() const
	{
		return *m_topBoneKeyFramList;
	}

	/*!
	*@brief	クリップ名を取得。
	*/
	const wchar_t* GetName() const
	{
		return m_clipName.c_str();
	}

	/*!
	*@brief	アニメーションイベントを取得。
	*/
	std::unique_ptr<AnimationEvent[]>& GetAnimationEvent()
	{
		return m_animationEvent;
	}
	/*!
	*@brief	アニメーションイベントの数を取得。
	*/
	int GetNumAnimationEvent() const
	{
		return m_numAnimationEvent;
	}

private:

	EnChangeAnimationClipUpAxis m_changeUpAxis = enNonChange;

	std::wstring m_clipName;	//!<アニメーションクリップの名前。

	bool m_isLoop = false;									//!<ループフラグ。
	std::vector<Keyframe*> m_keyframes;						//全てのキーフレーム。
	std::vector<keyFramePtrList> m_keyFramePtrListArray;	//ボーンごとのキーフレームのリストを管理するための配列。
															//例えば、m_keyFramePtrListArray[0]は0番目のボーンのキーフレームのリスト、
															//m_keyFramePtrListArray[1]は1番目のボーンのキーフレームのリストといった感じ。
	keyFramePtrList* m_topBoneKeyFramList = nullptr;

	std::unique_ptr<AnimationEvent[]>	m_animationEvent;			//アニメーションイベント。
	int									m_numAnimationEvent = 0;	//アニメーションイベントの数。
};

}