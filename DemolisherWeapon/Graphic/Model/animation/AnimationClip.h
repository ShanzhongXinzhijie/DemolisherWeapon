/*!
 *@brief	�A�j���[�V�����N���b�v�B
 */

#pragma once

namespace DemolisherWeapon {

//////////////////////////////////////////////////////////////////////
// �e��\����
//////////////////////////////////////////////////////////////////////
/*!
*@brief	�A�j���[�V�����N���b�v�̃w�b�_�[�B
*/
struct AnimClipHeader {
	std::uint32_t numKey;				//!<�L�[�t���[���̐��B
	std::uint32_t numAnimationEvent;	//!<�A�j���[�V�����C�x���g�̐��B
};
/*!
*@brief	�A�j���[�V�����C�x���g
*@�A�j���[�V�����C�x���g�͖��Ή��B
* ��肽�������玩���Ŏ�������悤�ɁB
*/
struct AnimationEventData {
	float	invokeTime;					//!<�A�j���[�V�����C�x���g���������鎞��(�P��:�b)
	std::uint32_t eventNameLength;		//!<�C�x���g���̒����B
};
/*!
*@brief	�L�[�t���[���B
*/
struct Keyframe {
	std::uint32_t boneIndex;	//!<�{�[���C���f�b�N�X�B
	float time;					//!<���ԁB
	CMatrix transform;			//!<�g�����X�t�H�[���B
};
/*!
*@brief	�L�[�t���[���B
*/
struct KeyframeRow {
	std::uint32_t boneIndex;	//!<�{�[���C���f�b�N�X�B
	float time;					//!<���ԁB
	CVector3 transform[4];		//!<�g�����X�t�H�[���B
};

//���ϊ��̎��
enum EnChangeAnimationClipUpAxis {
	enNonChange,
	enZtoY,
	enYtoZ,
};

/*!
*@brief	�A�j���[�V�����C�x���g�B
*/
class AnimationEvent {
public:
	AnimationEvent()
	{
	}
	/*!
	*@brief	�C�x���g�������Ԃ�ݒ�B
	*/
	float GetInvokeTime() const
	{
		return m_invokeTime;
	}
	/*!
	*@brief	�C�x���g�����擾�B
	*/
	const wchar_t* GetEventName() const
	{
		return m_eventName.c_str();
	}
	/*!
	*@brief	�C�x���g�������Ԃ�ݒ�B
	*/
	void SetInvokeTime(float time)
	{
		m_invokeTime = time;
	}
	/*!
	*@brief	�C�x���g����ݒ�B
	*/
	void SetEventName(const wchar_t* name)
	{
		m_eventName = name;
	}
	/*!
	*@brief	�C�x���g�������ς݂�����B
	*/
	/*bool IsInvoked() const
	{
		return m_isInvoked;
	}*/
	/*!
	*@brief	�C�x���g�������ς݂̃t���O��ݒ肷��B
	*/
	/*void SetInvokedFlag(bool flag)
	{
		m_isInvoked = flag;
	}*/
private:
	//bool m_isInvoked = false;	//!<�C�x���g�����ς݁H
	float m_invokeTime;			//!<�C�x���g�������ԁB
	std::wstring m_eventName;	//!<�C�x���g���B
};

//�^�C�v�ʂ��������炵���āA�������̂�std::vector<KeyFrame*>�̕ʖ���`�B
using keyFramePtrList = std::vector<Keyframe*>;

struct AnimationClipData {

	~AnimationClipData(){
		//�L�[�t���[����j���B
		for (auto& keyFrame : m_keyframes) {
			delete keyFrame;
		}
	}

	EnChangeAnimationClipUpAxis m_changeUpAxis = enNonChange;

	std::wstring m_clipName;	//!<�A�j���[�V�����N���b�v�̖��O�B
	std::wstring m_clipPass;	//�t�@�C���p�X

	std::vector<Keyframe*> m_keyframes;						//�S�ẴL�[�t���[���B
	std::vector<keyFramePtrList> m_keyFramePtrListArray;	//�{�[�����Ƃ̃L�[�t���[���̃��X�g���Ǘ����邽�߂̔z��B
															//�Ⴆ�΁Am_keyFramePtrListArray[0]��0�Ԗڂ̃{�[���̃L�[�t���[���̃��X�g�A
															//m_keyFramePtrListArray[1]��1�Ԗڂ̃{�[���̃L�[�t���[���̃��X�g�Ƃ����������B
	keyFramePtrList* m_topBoneKeyFramList = nullptr;

	std::unique_ptr<AnimationEvent[]>	m_animationEvent;			//�A�j���[�V�����C�x���g�B
	int									m_numAnimationEvent = 0;	//�A�j���[�V�����C�x���g�̐��B
};

class AnimationClipDataManager {
public:
	~AnimationClipDataManager() { Release(); }
	void Release() {
		//�}�b�v���̂��ׂĂ�AnimationClipData�����
		for (auto& p : m_animationClipDataMap) {
			delete p.second;
		}
	}

	//�V�K�쐬������A�߂�l=true
	bool Load(AnimationClipData*& returnACD, const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis = enNonChange);

private:
	typedef std::pair<int, int> key_t;

	struct key_hash
	{
		std::size_t operator()(const key_t& k) const
		{
			return k.first ^ k.second;
		}
	};

	std::unordered_map<key_t, AnimationClipData*, key_hash> m_animationClipDataMap;
};

/*!
*@brief	�A�j���[�V�����N���b�v�B
*/
class AnimationClip  {
public:
	/*!
	* @brief	�R���X�g���N�^
	*/
	AnimationClip() 
	{
	}
	/*!
	*@brief	�f�X�g���N�^�B
	*/
	~AnimationClip();
	/*!
	*@brief	�A�j���[�V�����N���b�v�����[�h�B
	*@param[in]	filePath	�t�@�C���p�X�B
	*/
	void Load(const wchar_t* filePath, bool loop = false, EnChangeAnimationClipUpAxis changeUpAxis = enNonChange);
	void Load(const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis) {
		Load(filePath, false, changeUpAxis);
	};
	
	/*!
	*@brief	���[�v����H
	*/
	bool IsLoop() const
	{
		return m_isLoop;
	}
	/*!
	*@brief	���[�v�t���O��ݒ肷��B
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
		return m_animationClipData->m_keyFramePtrListArray;
	}
	const keyFramePtrList& GetTopBoneKeyFrameList() const
	{
		return *m_animationClipData->m_topBoneKeyFramList;
	}

	/*!
	*@brief	�N���b�v�����擾�B
	*/
	const wchar_t* GetName() const
	{
		if (!m_animationClipData) { return nullptr; }
		return m_animationClipData->m_clipName.c_str();
	}

	//�t�@�C���p�X�̎擾
	const wchar_t* GetPass() const
	{
		if (!m_animationClipData) { return nullptr; }
		return m_animationClipData->m_clipPass.c_str();
	}

	/*!
	*@brief	�A�j���[�V�����C�x���g���擾�B
	*/
	const std::unique_ptr<AnimationEvent[]>& GetAnimationEvent() const
	{
		return m_animationClipData->m_animationEvent;
	}
	/*!
	*@brief	�A�j���[�V�����C�x���g�̐����擾�B
	*/
	int GetNumAnimationEvent() const
	{
		if (!m_animationClipData) { return 0; }
		return m_animationClipData->m_numAnimationEvent;
	}

private:
	AnimationClipData* m_animationClipData = nullptr;
	bool m_isLoop = false;									//!<���[�v�t���O�B

	static AnimationClipDataManager m_s_animationClipDataManager;
};

}