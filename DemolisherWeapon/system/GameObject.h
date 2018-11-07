#pragma once

namespace DemolisherWeapon {

class IGameObject;
class GameObjectManager;
class GOStatusReceiver;

//�Q�[���I�u�W�F�N�g�X�e�[�^�X
struct GOStatus {
	bool m_isDead = false;//���ɂ܂�����?
};
//�X�e�[�^�X�L���X�^�[
class GOStatusCaster {
public:
	GOStatusCaster(GOStatusReceiver* receiver) {
		m_receiver = receiver;
	}

	void ImDead() {
		m_alive = false;
	}
	bool GetAlive()const {
		return m_alive;
	}

	void Cast(const GOStatus& status);

private:
	GOStatusReceiver* m_receiver = nullptr;
	bool m_alive = true;
};
//�X�e�[�^�X���V�[�o�[
class GOStatusReceiver {
public:
	~GOStatusReceiver() {
		m_registerCaster->ImDead();
	}

	//�Ȃ񂩏����ݒ�
	void SetCaster(GOStatusCaster* caster) {
		m_registerCaster = caster;
	}
	void SetGameObject(IGameObject* go) {
		m_ptrGO = go;
	}

	//�X�e�[�^�X�X�V
	void SetStatus(const GOStatus& status) {
		m_status = status;
	}

//���������͎g��Ȃ���

//�������牺���g���Ă�������

	//�X�e�[�^�X���{��
	const GOStatus& GetStatus()const {
		return m_status;
	}

	//�X�e�[�^�X���̃Q�[���I�u�W�F�N�g�擾
	IGameObject* GetGameObject()const {
		return m_ptrGO;
	}

private:

	GOStatus m_status;
	IGameObject* m_ptrGO = nullptr;
	GOStatusCaster* m_registerCaster = nullptr;
};

//�Q�[���I�u�W�F�N�g�o�^�N���X
struct GORegister
{
	GORegister(bool enable, IGameObject* go) {
		isEnable = enable;
		gameObject = go;
	}

	bool isEnable = false;
	IGameObject* gameObject = nullptr;

private:
	bool m_nowOnHell = false;

	//�n���ɓ���
	void ArriveHell() {
		m_nowOnHell = true;
	}
	//�n���ɂ��邩?
	bool GetNowOnHell() const{
		return m_nowOnHell;
	}

//�@GameObjectManager���瑀��ł���	
	friend GameObjectManager;
};

//�Q�[���I�u�W�F�N�g
class IGameObject
{
public:
	IGameObject(bool isRegister = true);
	virtual ~IGameObject() {
		//�L���łȂ��񂾁I
		if (IsRegistered()) { m_register->isEnable = false; }
		//�X�e�[�^�X�X�V(���񂾂��I)
		m_status.m_isDead = true;
		CastStatus();
	};

private:

	//�J�n
	void SetIsStart() {
		m_isStart = true;
	}
	//�o�^
	void RegisterRegister(GORegister* regi) {
		m_register = regi;
	}

	//�X�e�[�^�X�����V�[�o�[�ɑ���
	void CastStatus() {
		auto it = m_statusCaster.begin();
		while (it != m_statusCaster.end()) {
			if ((*it).GetAlive()) {
				(*it).Cast(m_status);
				it++;
			}
			else {
				it = m_statusCaster.erase(it);//�폜
			}
		}
	} 

public:

	//�L����
	/*void SetEnable(bool e) {
		m_enable = e;
	}
	//�L���Ȃ̂��H
	bool GetEnable() {
		if (m_goToHell) { return false; }
		return m_enable;
	}*/

	//�J�n���Ă���̂��H
	bool GetIsStart() const{
		return m_isStart;
	}

	//����
	/*void LetsGoHell() {
		Rengoku();
		m_goToHell = true; m_enable = false;
	}
	//���O���ʂ̂��H
	bool GetGoToHell() {
		return m_goToHell;
	}
	//�n���ɓ���
	void ArriveHell() {
		m_nowOnHell = true;
	}
	//�n���ɂ��邩?
	bool GetNowOnHell() {
		return m_nowOnHell;
	}*/
	
	//�o�^����Ă��邩?
	bool IsRegistered() const{
		if (m_register) {
			return true;
		}
		return false;
	}

	//���W�X�^�[���擾
	GORegister* GetRegister()const {
		return m_register;
	}

	//�X�e�[�^�X���V�[�o�[��o�^
	void AddStatusReceiver(GOStatusReceiver* receiver) {
		m_statusCaster.emplace_back(receiver);
		receiver->SetStatus(m_status);
		receiver->SetCaster(&m_statusCaster.back());
		receiver->SetGameObject(this);
	}

public:

	virtual bool Start() { return true; };
	virtual void PreLoopUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};
	virtual void PostLoopUpdate() {};
	//virtual void Rengoku() {};

	virtual void PostRender() {};

private:
	//bool m_enable = true;
	bool m_isStart = false;

	//bool m_goToHell = false;//�n���Ɍ������Ă��邩?
	//bool m_nowOnHell = false;//�n���ɂ��邩?

	GORegister* m_register = nullptr;//�}�l�[�W���[�ɓo�^����Ă��邩(�|�C���^)
	
	GOStatus m_status;//���
	std::list<GOStatusCaster> m_statusCaster;//��Ԃ𑗐M����


//�@GameObjectManager���瑀��ł���	
	friend GameObjectManager;
};

class GameObjectManager {
public:

	void Start() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && !go.gameObject->GetIsStart()) {//go.gameObject->GetEnable() 
				if (go.gameObject->Start()) {
					go.gameObject->SetIsStart();
				}
			}
		}
	}
	void PreLoopUpdate() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetIsStart()) {
				go.gameObject->PreLoopUpdate();
			}
		}
	}
	void Update() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetIsStart()) {
				go.gameObject->Update();
			}
		}
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetIsStart()) {
				go.gameObject->PostUpdate();
			}
		}
	}
	void PostLoopUpdate() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetIsStart()) {
				go.gameObject->PostLoopUpdate();
			}
		}
	}
	void PostRender();

	//���̏���	
	/*void Hell() {
		auto it = m_gameObjectList.begin();
		while (it != m_gameObjectList.end()) {
			if ((*it)->GetGoToHell()) {
				if ((*it)->GetNowOnHell()) {//���ڂō폜
					(*it).reset();//delete (*it);
					it = m_gameObjectList.erase(it);
				}
				else {
					(*it)->ArriveHell();
					it++;
				}
			}
			else {
				it++;
			}
		}
	}*/
	void Hell() {
		auto it = m_gameObjectList.begin();
		while (it != m_gameObjectList.end()) {
			if (!(*it).isEnable) {				
				if ((*it).GetNowOnHell()) {//���ڂō폜
					it = m_gameObjectList.erase(it);//�폜
				}
				else {
					(*it).ArriveHell();
					it++;
				}
			}
			else {
				it++;
			}
		}
	}

public:

	//�Q�[���I�u�W�F�N�g�̓o�^
	void AddGameObj(IGameObject* go) {
		if (go == nullptr) { return; }

		//��d�o�^��h��
		if (go->IsRegistered()) { return; }

		m_gameObjectList.emplace_back(true, go);
		go->RegisterRegister(&m_gameObjectList.back());
	}

	//�Q�[���I�u�W�F�N�g�̍폜
	//void DeleteGameObj(IGameObject* go) {
	//	go->LetsGoHell();
	//}

private:

	std::list<GORegister> m_gameObjectList;
	//std::list< std::unique_ptr<IGameObject> > m_gameObjectList;

};

}