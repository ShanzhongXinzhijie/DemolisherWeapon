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
	class CCollisionObj;
}
struct RegColObj
{
	RegColObj(GameObj::CCollisionObj* p, int index) : m_CObj(p), m_index(index) {}

	int m_index = -1;
	bool m_isEnable = true;
	GameObj::CCollisionObj* m_CObj = nullptr;
};

namespace GameObj{

class CCollisionObj : public PhysicsBaseObject , public IGameObject {

public:

	//�R�[���o�b�N�֐��̈���
	struct SCallbackParam {
		const int m_nameKey;
		void* m_voidPtr;
		const btGhostObject&  m_ghostObject;
		IDW_Class* m_classPtr;

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

		//�|�C���^�����o��
		/*template<class T>
		T* GetGO() {
			return (T*)m_voidPtr;
		};*/
	};	

	/*static CCollisionObj* New(int lifespan = 1) {
		return new CCollisionObj(lifespan);
	}

private:*/
	CCollisionObj(int lifespanFrame = enNoTimer, const wchar_t* name = nullptr, IDW_Class* classPtr = nullptr, std::function<void(SCallbackParam&)> callbackFunction = nullptr, unsigned int group = 1, unsigned int mask = 0xFFFFFFFF)
	:
	m_group(group),m_mask(mask)
	{
		m_lifespan = max(lifespanFrame, enNoTimer);
		if (name) { SetName(name); }
		if (classPtr) { SetClass(classPtr); }
		if (callbackFunction) { SetCallback(callbackFunction); }
		Register();
	};

//private:
	~CCollisionObj() {
		if (m_isregistered) {
			m_register->m_isEnable = false;//�o�^���������Ƃ�
		}
		Release();
	};

public:
	void Release()override;

	//�폜����
	void Delete() {
		m_isDeath = true;
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
		m_isregistered = false;
	}

	void Update() override{
		if (m_lifespan != enNoTimer && m_lifespan <= 0 || m_isDeath) { delete this; return; }//if (Delete()) { return; } }
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
	void* GetData() { return m_void; };
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

private:
	
	/*!
	* @brief	�S�[�X�g�쐬�����̋��ʏ����B
	*/
	void CreateCommon(CVector3 pos, CQuaternion rot) override;

	//�}�l�[�W���[�ɓo�^����
	void Register();

private:
	RegColObj* m_register = nullptr;

	bool m_enable = true;//�L��?
	bool m_isDeath = false;//��?
	bool m_isInit = false;//�������ς�?
	bool m_isRegistPhysicsWorld = false;//!<�������[���h�ɓo�^���Ă��邩�ǂ����̃t���O�B
	bool m_isregistered = false;//�o�^�ς�?
	
	int m_lifespan = 0;//����

	unsigned int m_group = 0;
	unsigned int m_mask = 0;// 0xFFFFFFFF;//���ׂ�1

	int m_nameKey = 0;

	void* m_void = nullptr;
	IDW_Class* m_classPtr = nullptr;

	btGhostObject m_ghostObject;

	std::function<void(SCallbackParam&)> m_callback = nullptr;
};

class CollisionObjManager : public IGameObject{
public:

	//���菈�����
	void PostUpdate()override final;

	RegColObj* AddCollisionObj(CCollisionObj* obj) {
		m_colObjList.emplace_back(obj, m_colObjList.size());
		return &m_colObjList.back();
	};

private:

	std::list<RegColObj> m_colObjList;
};

}

}