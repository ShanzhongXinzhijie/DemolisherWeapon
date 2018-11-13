#pragma once

#include "../physics/PhysicsBaseObject.h"
#include "../util/Util.h"

namespace DemolisherWeapon{

class CCollisionObj : public PhysicsBaseObject , public IGameObject {

public:

	//�R�[���o�b�N�֐��̈���
	struct SCallbackParam {
		const int m_nameKey;
		void* m_class;
		const btGhostObject&  m_ghostObject;

		//���O����v���邩����
		bool EqualName(const wchar_t* name) {
			return m_nameKey == Util::MakeHash(name);
		}

		//�|�C���^�����o��
		template<class T>
		T* GetGO() {
			return (T*)m_class;
		};
	};

	/*static CCollisionObj* New(int lifespan = 1) {
		return new CCollisionObj(lifespan);
	}

private:*/
	CCollisionObj(int lifespan = 1, const wchar_t* name = nullptr, void* pointer = nullptr, std::function<void(SCallbackParam&)> callbackFunction = nullptr) {
		m_lifespan = lifespan;
		if (name) { SetName(name); }
		if (pointer) { SetPointer(pointer); }
		if (callbackFunction) { SetCallback(callbackFunction); }
		Register();
	};

private:
	~CCollisionObj() {
		Release();
	};

public:
	void Release()override;

	//�폜����
	void Delete() {
		//�f���[�g���W�X�g����Ă��疳��������
		if (m_isregistered) {
			m_isDeath = true;
		}
		else {
			delete this; return;
		}
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
	//�|�C���^��ݒ�
	void SetPointer(void* pointer) {
		m_void = pointer;
	}
	//������ݒ�
	void SetTimer(int lefttime) {
		m_lifespan = lefttime;
	}
	
	void PostUpdate()override {
		m_isregistered = false;
	}

	void Update() override{
		if (m_lifespan <= 0 || m_isDeath) { Delete(); }
		Register();
		m_lifespan--;
	}

	void RunCallback(SCallbackParam& param)
	{
		if (m_callback) {
			m_callback(param);
		}
	}

	//����I�u�W�F�N�g���擾
	btGhostObject& GetObj() { return m_ghostObject; }

	int GetNameKey()const { return m_nameKey; };
	void* GetData() { return m_void; };
	const btGhostObject& Getbt() { return m_ghostObject; };

	//���肵�Ă�������Ԃ�?
	bool IsEnable()const {
		return m_enable && m_isInit && !m_isDeath;
	}

private:
	
	/*!
	* @brief	�S�[�X�g�쐬�����̋��ʏ����B
	*/
	void CreateCommon(CVector3 pos, CQuaternion rot) override;

	//�}�l�[�W���[�ɓo�^����
	void Register();

private:
	bool m_enable = true;//�L��?
	bool m_isDeath = false;//��?
	bool m_isInit = false;//�������ς�?
	bool m_isRegistPhysicsWorld = false;//!<�������[���h�ɓo�^���Ă��邩�ǂ����̃t���O�B
	bool m_isregistered = false;//�o�^�ς�?

	int m_lifespan = 0;//����

	int m_group = 0;
	int m_mask;

	int m_nameKey = 0;

	void* m_void = nullptr;

	btGhostObject m_ghostObject;

	std::function<void(SCallbackParam&)> m_callback = nullptr;
};

class CollisionObjManager : public IGameObject{
public:

	//���菈�����
	void PostUpdate()override final;

	void AddCollisionObj(CCollisionObj* obj) {
		m_colObjList.push_back(obj);
	};

private:

	std::list<CCollisionObj*> m_colObjList;
};

}