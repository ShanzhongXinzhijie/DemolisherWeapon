#pragma once

#include "../physics/PhysicsBaseObject.h"
#include "../util/Util.h"

#define BIT(x) (1<<(x))

namespace DemolisherWeapon{

enum EnCollisionTimer {
	enNoTimer = -1,
};

static const int CCollisionObjFilter = 64;

namespace GameObj {
namespace Suicider {
	class CCollisionObj;
}
}
struct RegColObj
{
	RegColObj(GameObj::Suicider::CCollisionObj* p, int index) : m_CObj(p), m_index(index) {}

	int m_index = -1;
	bool m_isEnable = true;
	GameObj::Suicider::CCollisionObj* m_CObj = nullptr;
};

namespace GameObj{
namespace Suicider{

class CCollisionObj : public PhysicsBaseObject , public IGameObject {

public:

	//�R�[���o�b�N�֐��̈���
	struct SCallbackParam {
		//������������̏��
		const int m_nameKey;
		const wchar_t* m_name;
		void* m_voidPtr;
		const btGhostObject&  m_ghostObject;
		IDW_Class* m_classPtr;		
		const bool m_isA;
		const btManifoldPoint& m_contactPoint;//�ڐG�_�Ƃ����

		//���O����v���邩����
		bool EqualName(const wchar_t* name) {
			return m_nameKey == Util::MakeHash(name);
		}

		//�N���X�����o��
		template<class T>
		T* GetClass() {
			T* p = dynamic_cast<T*>(m_classPtr);
			if (p == nullptr) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "�N���X�̎��o��(�^�ϊ�)�Ɏ��s���܂����B\nSCallbackParam::GetClass()�̌^�����m�F���Ă��������B\n�ϊ���^��:%s\n�ϊ����^��:%s", typeid(T).name(), typeid(m_classPtr).name());
				MessageBox(NULL, message, "Error", MB_OK);
				std::abort();
#endif
			}
			return p;
		};
	};	

public:

	CCollisionObj(int lifespanFrame = enNoTimer, const wchar_t* name = nullptr, IDW_Class* classPtr = nullptr, std::function<void(SCallbackParam&)> callbackFunction = nullptr, unsigned int group = 1, unsigned int mask = 0xFFFFFFFF)
	:
	m_group(group),m_mask(mask)
	{
		m_lifespan = max(lifespanFrame, enNoTimer);
		if (name) { SetName(name); }
		if (classPtr) { SetClass(classPtr); }
		if (callbackFunction) { SetCallback(callbackFunction); }
		Register(true);
	};

	~CCollisionObj() {
#ifndef DW_MASTER
		if (m_isHanteing) {
			//���蒆
			MessageBox(NULL, "���蒆��CCollisionObj���폜����܂����B\n��߂Ȃ����B\nCCollisionObj::Delete��DeleteGO���g���΂����񂶂�Ȃ�����", "Error", MB_OK);
			std::abort();
		}
#endif

		if (m_isregistered) {
			m_register->m_isEnable = false;//�o�^���������Ƃ�
		}
		Release();
	};

public:

	//�폜����
	void Delete() {
		m_isDeath = true;
	}
	//���̃��[�v�ō폜����(���̃��[�v�ł͔��肳��܂���)
	void DeleteNextLoop() {
		m_killMark = true;
	}

	//�L���E�����̐ݒ�
	void SetEnable(bool enable){
		m_enable = enable;
	}
	bool GetEnable()const {
		return m_enable;
	}

	/*!
	* @brief	���W��ݒ�B
	*/
	void SetPosition(const CVector3& pos)
	{
		auto& btTrans = m_ghostObject.getWorldTransform();
		btVector3 btPos;
		pos.CopyTo(btPos);
		btTrans.setOrigin(btPos);
	}
	/*!
	* @brief	��]��ݒ�B
	*/
	void SetRotation(const CQuaternion& rot)
	{
		auto& btTrans = m_ghostObject.getWorldTransform();
		btQuaternion btRot;
		rot.CopyTo(btRot);
		btTrans.setRotation(btRot);
	}

	//���O��ݒ�
	void SetName(const wchar_t* name) {
		m_nameKey = Util::MakeHash(name);
		m_name = name;
	}
	const wchar_t* GetName()const {
		return m_name.c_str();
	}
	//�R�[���o�b�N�֐���ݒ�
	void SetCallback(std::function<void(SCallbackParam&)> callbackFunction) {
		m_callback = callbackFunction;
	}
	//�N���X�̃|�C���^��ݒ�
	void SetClass(IDW_Class* classPtr) {
		m_classPtr = classPtr;
	}
	//������ݒ�
	void SetTimer(int lefttimeFrame) {
		m_lifespan = max(lefttimeFrame, enNoTimer);
	}
	int GetTimer()const {
		return m_lifespan;
	}

	//�|�C���^��ݒ�
	void SetPointer(void* pointer) {
		m_void = pointer;
	}

	//�O���[�v��ݒ�
	void SetGroup(unsigned int group) {
		m_group = group;
	}
	//�}�X�N��ݒ�
	void SetMask(unsigned int mask) {
		m_mask = mask;
	}

	//�O���[�v�̎w��̃r�b�g���I���ɂ���
	void On_OneGroup(unsigned int oneGroup) {
		m_group = m_group | BIT(oneGroup);
	}
	//�}�X�N�̎w��̃r�b�g���I���ɂ���
	void On_OneMask(unsigned int oneMask) {
		m_mask = m_mask | BIT(oneMask);
	}
	//�O���[�v�̎w��̃r�b�g���I�t�ɂ���
	void Off_OneGroup(unsigned int oneGroup) {
		m_group = m_group & ~BIT(oneGroup);
	}
	//�}�X�N�̎w��̃r�b�g���I�t�ɂ���
	void Off_OneMask(unsigned int oneMask) {
		m_mask = m_mask & ~BIT(oneMask);
	}

	//���ׂẴO���[�v�ɑ�����悤�ݒ�
	void All_On_Group() {
		m_group = 0xFFFFFFFF;
	}
	//���ׂẴO���[�v�Ɣ��肷��悤�}�X�N��ݒ�
	void All_On_Mask() {
		m_mask = 0xFFFFFFFF;
	}
	//�ǂ̃O���[�v�ɂ������Ȃ��悤�ݒ�
	void All_Off_Group() {
		m_group = 0;
	}
	//�ǂ̃O���[�v�Ƃ����肵�Ȃ��悤�}�X�N��ݒ�
	void All_Off_Mask() {
		m_mask = 0;
	}

	void PostUpdate()override {
		//���s����CollisionObjManager���x������ł��鏈��
		m_isregistered = false;
		m_register = nullptr;
	}

	void Update() override{
		if (m_lifespan != enNoTimer && m_lifespan <= 0 || m_isDeath || m_killMark) { delete this; return; }
		Register();
		if (m_lifespan != enNoTimer) { m_lifespan--; }
	}

	void RunCallback(SCallbackParam& param)
	{
		if (m_callback) {
			m_callback(param);
		}
	}

	//����I�u�W�F�N�g���擾
	btGhostObject& GetCollisionObject() { return m_ghostObject; }

	int GetNameKey()const { return m_nameKey; };
	void* GetPointer() { return m_void; };
	IDW_Class* GetClass() { return m_classPtr; };

	unsigned int GetGroup()const { return m_group; }
	unsigned int GetMask() const { return m_mask; }

	//���肵�Ă�������Ԃ�?
	bool IsEnable()const {
		return m_enable && m_isInit && !m_isDeath;
	}

	int GetIndex()const {
		if (m_register) {
			return m_register->m_index;
		}
		return -1;
	}

	//RegColObj* GetRegister() { return m_register; }

	//���蒆���̃t���O
	void SetHanteing(bool flag) {
		m_isHanteing = flag;
	}
	bool GetHanteing()const {
		return m_isHanteing;
	}

private:

	void Release()override;

	/*!
	* @brief	�S�[�X�g�쐬�����̋��ʏ����B
	*/
	void CreateCommon(CVector3 pos, CQuaternion rot) override;

	//�}�l�[�W���[�ɓo�^����
	void Register(bool compulsion = false);

private:
	RegColObj* m_register = nullptr;

	bool m_enable = true;//�L��?
	bool m_isDeath = false;//��?
	bool m_killMark = false;//�������߂�?
	bool m_isInit = false;//�������ς�?
	bool m_isRegistPhysicsWorld = false;//!<�������[���h�ɓo�^���Ă��邩�ǂ����̃t���O�B
	bool m_isregistered = false;//�o�^�ς�?

	bool m_isHanteing = false; //���蒆��?
	
	int m_lifespan = 0;//����

	unsigned int m_group = 0;
	unsigned int m_mask = 0;// 0xFFFFFFFF;//���ׂ�1

	int m_nameKey = 0;
	std::wstring m_name;

	void* m_void = nullptr;
	IDW_Class* m_classPtr = nullptr;

	btGhostObject m_ghostObject;

	std::function<void(SCallbackParam&)> m_callback = nullptr;
};

}

class CollisionObjManager : public IGameObject{
public:

	//���菈�����
	void PostUpdate()override final;

	RegColObj* AddCollisionObj(Suicider::CCollisionObj* obj) {
		m_colObjList.emplace_back(obj, m_colObjList.size());
		return &m_colObjList.back();
	};

private:

	std::list<RegColObj> m_colObjList;
};

}

}