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
	/*bool IsInvoked() const
	{
		return m_isInvoked;
	}*/
	/*!
	*@brief	イベントが発生済みのフラグを設定する。
	*/
	/*void SetInvokedFlag(bool flag)
	{
		m_isInvoked = flag;
	}*/
private:
	//bool m_isInvoked = false;	//!<イベント発生済み？
	float m_invokeTime;			//!<イベント発生時間。
	std::wstring m_eventName;	//!<イベント名。
};

//タイプ量が長ったらしくて、うざいのでstd::vector<KeyFrame*>の別名定義。
using keyFramePtrList = std::vector<Keyframe*>;

struct AnimationClipData {

	~AnimationClipData(){
		//キーフレームを破棄。
		for (auto& keyFrame : m_keyframes) {
			delete keyFrame;
		}
	}

	EnChangeAnimationClipUpAxis m_changeUpAxis = enNonChange;

	std::wstring m_clipName;	//!<アニメーションクリップの名前。
	std::wstring m_clipPass;	//ファイルパス

	std::vector<Keyframe*> m_keyframes;						//全てのキーフレーム。
	std::vector<keyFramePtrList> m_keyFramePtrListArray;	//ボーンごとのキーフレームのリストを管理するための配列。
															//例えば、m_keyFramePtrListArray[0]は0番目のボーンのキーフレームのリスト、
															//m_keyFramePtrListArray[1]は1番目のボーンのキーフレームのリストといった感じ。
	keyFramePtrList* m_topBoneKeyFramList = nullptr;

	std::unique_ptr<AnimationEvent[]>	m_animationEvent;			//アニメーションイベント。
	int									m_numAnimationEvent = 0;	//アニメーションイベントの数。
};

class AnimationClipDataManager {
public:
	~AnimationClipDataManager() { Release(); }
	void Release() {
		//マップ内のすべてのAnimationClipDataを解放
		for (auto& p : m_animationClipDataMap) {
			delete p.second;
		}
	}

	//新規作成したら、戻り値=true
	bool Load(AnimationClipData*& returnACD, const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis = enNonChange);

private:
	typedef std::pair<int, int> key_t;

	struct key_hash
	{
		std::size_t operator()(const key_t& k) const
		{
			return Util::HashCombine(k.first, k.second);
		}
	};

	std::unordered_map<key_t, AnimationClipData*, key_hash> m_animationClipDataMap;
};

/*!
*@brief	アニメーションクリップ。
*/
class AnimationClip  {
public:
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
	void Load(const wchar_t* filePath, bool loop = false, EnChangeAnimationClipUpAxis changeUpAxis = enNonChange, float startTimeOffset = 0.0f);
	void Load(const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis) {
		Load(filePath, false, changeUpAxis);
	};
	void Load(const wchar_t* filePath, bool loop, float startTimeOffset) {
		Load(filePath, loop, enNonChange, startTimeOffset);
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

	//開始時間を取得
	float GetStartTimeOffset()const {
		return m_startTimeOffset;
	}

	/*!
	*@brief
	*/
	const std::vector<keyFramePtrList>& GetKeyFramePtrListArray() const
	{
		return m_animationClipData->m_keyFramePtrListArray;
	}
	const keyFramePtrList& GetTopBoneKeyFrameList() const
	{
		return *m_animationClipData->m_topBoneKeyFramList;
	}

	/*!
	*@brief	クリップ名を取得。
	*/
	const wchar_t* GetName() const
	{
		if (!m_animationClipData) { return nullptr; }
		return m_animationClipData->m_clipName.c_str();
	}

	//ファイルパスの取得
	const wchar_t* GetPass() const
	{
		if (!m_animationClipData) { return nullptr; }
		return m_animationClipData->m_clipPass.c_str();
	}

	//ハッシュ値の取得
	std::size_t GetHash()const{
		if (!m_animationClipData) { return -1; }
		
		std::size_t hash = Util::MakeHash(GetPass());
		hash = Util::HashCombine(hash, std::hash<bool>()(m_isLoop));
		hash = Util::HashCombine(hash, std::hash<float>()(m_startTimeOffset));

		return hash;
	}

	/*!
	*@brief	アニメーションイベントを取得。
	*/
	const std::unique_ptr<AnimationEvent[]>& GetAnimationEvent() const
	{
		return m_animationClipData->m_animationEvent;
	}
	/*!
	*@brief	アニメーションイベントの数を取得。
	*/
	int GetNumAnimationEvent() const
	{
		if (!m_animationClipData) { return 0; }
		return m_animationClipData->m_numAnimationEvent;
	}

private:
	AnimationClipData* m_animationClipData = nullptr;
	bool m_isLoop = false;									//!<ループフラグ。
	float m_startTimeOffset = 0.0f;

	static AnimationClipDataManager m_s_animationClipDataManager;
};

}