#pragma once
#include <unordered_map>
#include "../util/Util.h"

namespace DemolisherWeapon {

class IGameObject;
class GameObjectManager;
class GONewDeleteManager;
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
	~GOStatusCaster();

	void ImDead() {
		m_alive = false;
	}
	bool GetAlive()const {
		return m_alive;
	}

	void Cast(const GOStatus& status);

	const GOStatusReceiver* const GetReceiver() const{
		return m_receiver;
	}

private:
	GOStatusReceiver* m_receiver = nullptr;
	bool m_alive = true;
};
//�X�e�[�^�X���V�[�o�[
class GOStatusReceiver {
public:
	~GOStatusReceiver() {
		if(m_registerCaster) m_registerCaster->ImDead();
	}

	//�Ȃ񂩏����ݒ�
	//���[�U�[�͎g��Ȃ��ł�������
	void SetCaster(GOStatusCaster* caster) {
		m_registerCaster = caster;
	}
	//���[�U�[�͎g��Ȃ��ł�������
	void SetGameObject(IGameObject* go) {
		m_ptrGO = go;
	}

	//�X�e�[�^�X�X�V
	//���[�U�[�͎g��Ȃ��ł�������
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

class GODeathListener;
//���S���X�i�[�o�^�N���X
struct GODeathListenerRegister {
	GODeathListenerRegister(GODeathListener* listener_ptr) {
		listener = listener_ptr;
	}
	~GODeathListenerRegister();

	bool enable = true;
	GODeathListener* listener = nullptr;
};
//���S���X�i�[
class GODeathListener {
public:
	~GODeathListener() {
		if (m_resister) { m_resister->enable = false; }
	}

	//�f�X���X�i�[�̈���
	struct SDeathParam {
		IGameObject* gameObject = nullptr;
	};	

	//���S�ʒm���Ɏ��s�����֐���ݒ�
	void SetFunction(std::function<void(const SDeathParam& param)> func) {
		m_function = func;
	}

	//���[�U�[�͎g��Ȃ��ł�������
	void SetResister(GODeathListenerRegister* resister) {
		m_resister = resister;
	}
	//���[�U�[�͎g��Ȃ��ł�������
	void RunFunction(const SDeathParam& param) {
		m_function(param);
	}

private:
	std::function<void(const SDeathParam& param)> m_function;
	GODeathListenerRegister* m_resister = nullptr;
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

class IDW_Class {
public:
	virtual ~IDW_Class() {};
};

//�Q�[���I�u�W�F�N�g
class IGameObject : public IDW_Class
{
public:
	IGameObject(bool isRegister = true, bool quickStart = false);
	virtual ~IGameObject();

	IGameObject(const IGameObject& go) = delete;//�R�s�[�R���X�g���N�^
	IGameObject& operator=(const IGameObject&) = delete;

	IGameObject(IGameObject&&)noexcept = delete;// {};//���[�u�R���X�g���N�^
	IGameObject& operator=(IGameObject&&)noexcept = delete;// {};

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
		auto endit = m_statusCaster.end();
		while (it != endit) {
			if ((*it).GetAlive()) {
				(*it).Cast(m_status);
				it++;
			}
			else {
				it = m_statusCaster.erase(it);//�폜
			}
		}
	} 

	//NewGO�ō�����Ƃ����}�[�N����
	void MarkNewGOMark() {
		m_newgoMark = true;
	}
	//DeleteGO���ꂽ
	void ���O�͂�������ł���() {
		m_isDead = true;
	}

public:
	//NewGO�ō������?
	bool GetNewGOMark() const {
		return m_newgoMark;
	}
	//DeleteGO����Ă�?
	bool ���O�͂�������ł���H() const{
		return m_isDead;
	}

	//�L����
	void SetEnable(bool e){
		m_enable = e;
	}
	//�L���Ȃ̂��H
	bool GetEnable() const{
		//if (m_goToHell) { return false; }
		return m_enable && !m_isDead;
	}

	//���z�֐������̗񋓎q
	enum VirtualFuncs {
		enStart,
		enPreLoopUpdate,
		enPreUpdate,
		enUpdate,
		enPostUpdate,
		enPostLoopUpdate,
		enPostLoopPostUpdate,
		enPre3DRender,
		enHUDRender,
		enPostRender,
		enVirtualFuncNum
	};
	//���z�֐����I�[�o�[���C�h����Ă��邩?
	bool GetIsOverrideVFunc(VirtualFuncs funcType)const {
		return m_isRunFunc[funcType];
	}

	//�J�n���Ă���̂��H
	bool GetIsStart(){
		if (m_quickStart) {
			if (GetEnable() && !m_isStart) {
				if (Start()) {
					SetIsStart();
				}
			}
		}
		return m_isStart;
	}
	
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
	//�X�e�[�^�X���V�[�o�[�o�^����
	void RemoveStatusReceiver(GOStatusReceiver* receiver) {
		for (auto& C : m_statusCaster) {
			if (C.GetReceiver() == receiver) {
				C.ImDead();
			}
		}
	}

	//���S���X�i�[�o�^
	void AddDeathListener(GODeathListener* listener)
	{
		m_deathListeners.emplace_back(listener);
		listener->SetResister(&m_deathListeners.back());
	}
	//���S���X�i�[�o�^����
	void RemoveDeathListener(GODeathListener* listener) {
		for (auto& R : m_deathListeners) {
			if (R.listener == listener) {
				R.enable = false;
			}
		}
	}

	//���O������
	void SetName(const wchar_t* objectName);

private:
	//���z�֐��̎��s����߂�
	void OffIsRunVFunc(VirtualFuncs type);

public:
	//���z�֐�

	//�����J�n���Ɏ��s
	//�߂�l��false���Ə����J�n���Ȃ�
	virtual bool Start() {
		OffIsRunVFunc(enStart);
		return true; 
	}

	//�Q�[�����[�v�O�Ɏ��s
	virtual void PreLoopUpdate() {
		OffIsRunVFunc(enPreLoopUpdate);
	}
	
	//�Q�[�����[�v���Ŏ��s
	virtual void PreUpdate() {
		OffIsRunVFunc(enPreUpdate);
	}
	virtual void Update() {
		OffIsRunVFunc(enUpdate);
	}
	virtual void PostUpdate() {
		OffIsRunVFunc(enPostUpdate);
	}

	//�Q�[�����[�v��Ɏ��s
	virtual void PostLoopUpdate() {
		OffIsRunVFunc(enPostLoopUpdate);
	}
	virtual void PostLoopPostUpdate() {
		OffIsRunVFunc(enPostLoopPostUpdate);
	}

	//3D�`��O�Ɏ��s(��ʂ���)
	//int num ���s���̉�ʔԍ�
	virtual void Pre3DRender(int num) {
		OffIsRunVFunc(enPre3DRender);
	}

	//���̊֐�����HUD��2D�O���t�B�b�N��`��
	//int HUDNum �`��Ώۂ�HUD�̔ԍ�
	virtual void HUDRender(int HUDNum) {
		OffIsRunVFunc(enHUDRender);
	}

	//2D�O���t�B�b�N�����̊֐����ŕ`�悵�Ă�������
	//��CFont,CSprite�Ȃ�
	virtual void PostRender() {
		OffIsRunVFunc(enPostRender);
	}

private:
	bool m_isDead = false;//�������S
	bool m_enable = true;
	bool m_isStart = false;
	bool m_quickStart = false;

	bool m_isRunFunc[enVirtualFuncNum];//���z�֐����I�[�o�[���C�h����Ă��邩?(���s���邩)

	GORegister* m_register = nullptr;//�}�l�[�W���[�ɓo�^����Ă��邩(�|�C���^)
	
	GOStatus m_status;//���
	std::list<GOStatusCaster> m_statusCaster;//��Ԃ𑗐M����
	
	std::list<GODeathListenerRegister> m_deathListeners;//���S���X�i�[����B

	bool m_newgoMark = false;//NewGO�ō������?

//�@GameObjectManager���瑀��ł���	
	friend GameObjectManager;
	friend GONewDeleteManager;
};

//�����œo�^�����Ȃ��Q�[���I�u�W�F�N�g
class INRGameObject : public IGameObject{
public:
	INRGameObject(bool quickStart = false) : IGameObject(false, quickStart) {};
	virtual ~INRGameObject() {};
};

//�ł�����葁��������J�n����Q�[���I�u�W�F�N�g
class IQSGameObject : public IGameObject {
public:
	IQSGameObject(bool isRegister = true) : IGameObject(isRegister, true) {};
	virtual ~IQSGameObject() {};
};

//�Q�[���I�u�W�F�N�g�̃}�l�[�W���[
//�Q�[���I�u�W�F�N�g��Update�Ƃ��Ăяo��
class GameObjectManager {
public:
	~GameObjectManager() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable) { go.gameObject->RegisterRegister(nullptr); }
		}
	}

	void Start();
	void PreLoopUpdate();
	void Update();
	void PostLoopUpdate();
	void Pre3DRender(int num);
	void HUDRender(int HUDNum);
	void PostRender();

	//���̏���
	void Hell();

private:
	//�֐����s���X�g����Q�[���I�u�W�F�N�g�Q�Ƃ��폜
	void DeleteFromFuncList();

public:
	//���̃t���[������GO���폜���ꂽ
	void EnableIsDeleteGOThisFrame() {
		m_isDeleteGOThisFrame = true;
	}
	//���̃t���[�����ɉ��z�֐��̎��s�m�F�����ꂽ
	void EnableIsCheckVFuncThisFrame(IGameObject::VirtualFuncs ind) {
		m_isCheckVFuncThisFrame[ind] = true;
	}

	//�Q�[���I�u�W�F�N�g�̓o�^
	void AddGameObj(IGameObject* go) {
		if (go == nullptr) { return; }

		//��d�o�^��h��
		if (go->IsRegistered()) { return; }

		//�Q�[���I�u�W�F�N�g�����X�g�o�^
		m_gameObjectList.emplace_back(true, go);
		//�Q�[���I�u�W�F�N�g��GORegister��o�^
		go->RegisterRegister(&m_gameObjectList.back());

		//���s�֐����X�g�փQ�[���I�u�W�F�N�g��o�^
		for (auto& list : m_runFuncGOList) {
			list.emplace_back(&m_gameObjectList.back());
		}
	}

	//�Q�[���I�u�W�F�N�g�ɖ��O������
	void SetNameGO(IGameObject* go, const wchar_t* objectName) {
		if (!go) { return; }

		GORegister* regiGo = go->GetRegister();
		if (regiGo) {
			int nameKey = Util::MakeHash(objectName);			
			m_gameObjectMap.emplace(nameKey, regiGo);
		}
		else {
#ifndef DW_MASTER
			MessageBox(NULL, "�o�^����Ă��Ȃ��Q�[���I�u�W�F�N�g�ɖ��O�����悤�Ƃ��Ă��܂�", "Error", MB_OK);
			std::abort();
#endif
		}
	}

	//�Q�[���I�u�W�F�N�g�̌���
	template<class T>
	T* FindGO(const wchar_t* objectName)
	{
		auto range = m_gameObjectMap.equal_range(Util::MakeHash(objectName));

		for (auto it = range.first; it != range.second; ++it) {
			auto& regiGo = it->second;

			//�L�����H
			if (regiGo->isEnable && regiGo->gameObject->GetEnable()) {
				//�������B
				T* p = dynamic_cast<T*>(regiGo->gameObject);
				if (p != nullptr) {
					return p;
				}
			}
		}

		return nullptr;
	}
	template<class T>
	T* FindGO()
	{
		for (auto& regiGo : m_gameObjectList) {
			//�L�����H
			if (regiGo.isEnable && regiGo.gameObject->GetEnable()) {
				//�������B
				T* p = dynamic_cast<T*>(regiGo.gameObject);
				if (p != nullptr) {
					return p;
				}
			}
		}

		return nullptr;
	}
	template<class T>
	void QueryGOs(const wchar_t* objectName, std::function<bool(T* go)> func)
	{
		auto range = m_gameObjectMap.equal_range(Util::MakeHash(objectName));

		for (auto it = range.first; it != range.second; ++it){
			auto& regiGo = it->second;		

			//�L�����H
			if (regiGo->isEnable && regiGo->gameObject->GetEnable()) {
				//�������B
				T* p = dynamic_cast<T*>(regiGo->gameObject);
				if (p != nullptr) {
					if (func(p) == false) {
						//�N�G�����f�B
						return;
					}
				}
			}
		}
	}
	template<class T>
	void QueryGOs(std::function<bool(T* go)> func)
	{
		for (auto& regiGo : m_gameObjectList) {
			//�L�����H
			if (regiGo.isEnable && regiGo.gameObject->GetEnable()) {
				//�������B
				T* p = dynamic_cast<T*>(regiGo.gameObject);
				if (p != nullptr) {
					if (func(p) == false) {
						//�N�G�����f�B
						return;
					}
				}
			}
		}
	}

private:
	std::list<GORegister> m_gameObjectList;//�Q�[���I�u�W�F�N�g�̃��X�g
	std::list<GORegister*> m_runFuncGOList[IGameObject::enVirtualFuncNum];//�e�֐������s����Q�[���I�u�W�F�N�g�̃��X�g
	std::unordered_multimap<int, GORegister*> m_gameObjectMap;//���O�t���Q�[���I�u�W�F�N�g�̎���

	bool m_isDeleteGOThisFrame = false;//���̃t���[������GO���폜���ꂽ��?
	bool m_isCheckVFuncThisFrame[IGameObject::enVirtualFuncNum] = {};//���̃t���[�����ɉ��z�֐��̎��s�m�F�����ꂽ��?
};

//�Q�[���I�u�W�F�N�g�̐����ƍ폜�̃}�l�[�W���[
class GONewDeleteManager {
public:
	template<class T, class... TArgs>
	T*  NewGO(TArgs... ctorArgs) {
		//new����+�t���O����
		T* newObject = new T(ctorArgs...);
		newObject->MarkNewGOMark();
		return newObject;
	}

	//(�Q�[���I�u�W�F�N�g�̖������t���O�����B���ۂɃC���X�^���X���폜�����̂́A�S�Ă�GO��PostUpdate���I����Ă���)
	bool DeleteGO(IGameObject* gameObject, bool newgoCheck, bool instantKill = false) {
		
		if (!gameObject) { return false; }

		if (newgoCheck && !gameObject->GetNewGOMark()) {
			//Newgo�̃t���O�����ĂȂ�������G���[
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "NewGO����Ă��Ȃ��Q�[���I�u�W�F�N�g��DeleteGO���悤�Ƃ��Ă��܂��B\n�^��:%s", typeid(gameObject).name());
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
		}
		else {
			if (!gameObject->���O�͂�������ł���H()) {//�܂��E����Ă��Ȃ�
				//�����ɎE��
				if (instantKill) {
					delete gameObject;
					return true;
				}
				
				//���ƂŎE��
				//������
				gameObject->���O�͂�������ł���();
				//�E�����X�g�o�^
				m_deleteList.emplace_back(gameObject);
				return true;
			}
		}

		return false;
	}

	void FarewellDearDeadman() {
		//�E��
		for (auto& GO : m_deleteList) {
			delete GO;
		}
		m_deleteList.clear();
	}

private:
	std::list<IGameObject*> m_deleteList;
};

}