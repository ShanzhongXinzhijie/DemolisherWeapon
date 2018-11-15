#pragma once

namespace DemolisherWeapon {

class IGameObject;
class GameObjectManager;
class GONewDeleteManager;
class GOStatusReceiver;

//ゲームオブジェクトステータス
struct GOStatus {
	bool m_isDead = false;//死にましたか?
};
//ステータスキャスター
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
//ステータスレシーバー
class GOStatusReceiver {
public:
	~GOStatusReceiver() {
		m_registerCaster->ImDead();
	}

	//なんか初期設定
	void SetCaster(GOStatusCaster* caster) {
		m_registerCaster = caster;
	}
	void SetGameObject(IGameObject* go) {
		m_ptrGO = go;
	}

	//ステータス更新
	void SetStatus(const GOStatus& status) {
		m_status = status;
	}

//ここから上は使わないで

//ここから下を使ってください

	//ステータスを閲覧
	const GOStatus& GetStatus()const {
		return m_status;
	}

	//ステータス元のゲームオブジェクト取得
	IGameObject* GetGameObject()const {
		return m_ptrGO;
	}

private:

	GOStatus m_status;
	IGameObject* m_ptrGO = nullptr;
	GOStatusCaster* m_registerCaster = nullptr;
};

//ゲームオブジェクト登録クラス
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

	//地獄に到着
	void ArriveHell() {
		m_nowOnHell = true;
	}
	//地獄にいるか?
	bool GetNowOnHell() const{
		return m_nowOnHell;
	}

//　GameObjectManagerから操作できる	
	friend GameObjectManager;
};

class IDW_Class {
public:
	virtual ~IDW_Class() {};
};

//ゲームオブジェクト
class IGameObject : public IDW_Class
{
public:

	//デスリスナーの引数
	struct SDeathParam {
		IGameObject* gameObject = nullptr;
	};

public:
	IGameObject(bool isRegister = true);
	virtual ~IGameObject() {
		//有効でないんだ！
		if (IsRegistered()) { m_register->isEnable = false; }
		//ステータス更新(死んだぞ！)
		m_status.m_isDead = true;
		CastStatus();
		//デスリスナーに通知
		SDeathParam param;
		param.gameObject = this;
		for (auto& listener : m_deathListeners) {
			listener(param);
		}
	};

	IGameObject(const IGameObject& go) = delete;//コピーコンストラクタ
	IGameObject& operator=(const IGameObject&) = delete;

	IGameObject(IGameObject&&)noexcept = delete;// {};//ムーブコンストラクタ
	IGameObject& operator=(IGameObject&&)noexcept = delete;// {};

private:

	//開始
	void SetIsStart() {
		m_isStart = true;
	}
	//登録
	void RegisterRegister(GORegister* regi) {
		m_register = regi;
	}

	//ステータスをレシーバーに送る
	void CastStatus() {
		auto it = m_statusCaster.begin();
		while (it != m_statusCaster.end()) {
			if ((*it).GetAlive()) {
				(*it).Cast(m_status);
				it++;
			}
			else {
				it = m_statusCaster.erase(it);//削除
			}
		}
	} 

	//NewGOで作ったというマークつける
	void MarkNewGOMark() {
		m_newgoMark = true;
	}

public:

	//有効化
	/*void SetEnable(bool e) {
		m_enable = e;
	}
	//有効なのか？
	bool GetEnable() {
		if (m_goToHell) { return false; }
		return m_enable;
	}*/

	//開始しているのか？
	bool GetIsStart() const{
		return m_isStart;
	}

	//死ぬ
	/*void LetsGoHell() {
		Rengoku();
		m_goToHell = true; m_enable = false;
	}
	//お前死ぬのか？
	bool GetGoToHell() {
		return m_goToHell;
	}
	//地獄に到着
	void ArriveHell() {
		m_nowOnHell = true;
	}
	//地獄にいるか?
	bool GetNowOnHell() {
		return m_nowOnHell;
	}*/
	
	//登録されているか?
	bool IsRegistered() const{
		if (m_register) {
			return true;
		}
		return false;
	}

	//レジスターを取得
	GORegister* GetRegister()const {
		return m_register;
	}

	//ステータスレシーバーを登録
	void AddStatusReceiver(GOStatusReceiver* receiver) {
		m_statusCaster.emplace_back(receiver);
		receiver->SetStatus(m_status);
		receiver->SetCaster(&m_statusCaster.back());
		receiver->SetGameObject(this);
	}

	//死亡リスナー登録
	void AddDeathListener(std::function<void(const SDeathParam& param)> listener)
	{
		m_deathListeners.push_back(listener);
	}

	//NewGOで作ったか?
	bool GetNewGOMark() const{
		return m_newgoMark;
	}

public:

	virtual bool Start() { return true; };
	virtual void PreLoopUpdate() {};
	virtual void PreUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};
	virtual void PostLoopUpdate() {};
	//virtual void Rengoku() {};

	virtual void PostRender() {};

private:
	//bool m_enable = true;
	bool m_isStart = false;

	//bool m_goToHell = false;//地獄に向かっているか?
	//bool m_nowOnHell = false;//地獄にいるか?

	GORegister* m_register = nullptr;//マネージャーに登録されているか(ポインタ)
	
	GOStatus m_status;//状態
	std::list<GOStatusCaster> m_statusCaster;//状態を送信する

	std::list<std::function<void(const SDeathParam& param)>> m_deathListeners;//死亡リスナーさん達

	bool m_newgoMark = false;//NewGOで作ったか?

//　GameObjectManagerから操作できる	
	friend GameObjectManager;
	friend GONewDeleteManager;
};

//自動で登録をしないゲームオブジェクト
class INRGameObject : public IGameObject{
public:
	INRGameObject() : IGameObject(false) {};
	virtual ~INRGameObject() {};
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
				go.gameObject->PreUpdate();
			}
		}
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

	//死の処理	
	/*void Hell() {
		auto it = m_gameObjectList.begin();
		while (it != m_gameObjectList.end()) {
			if ((*it)->GetGoToHell()) {
				if ((*it)->GetNowOnHell()) {//二回目で削除
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
				if ((*it).GetNowOnHell()) {//二回目で削除
					it = m_gameObjectList.erase(it);//削除
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

	//ゲームオブジェクトの登録
	void AddGameObj(IGameObject* go) {
		if (go == nullptr) { return; }

		//二重登録を防ぐ
		if (go->IsRegistered()) { return; }

		m_gameObjectList.emplace_back(true, go);
		go->RegisterRegister(&m_gameObjectList.back());
	}

	//ゲームオブジェクトの削除
	//void DeleteGameObj(IGameObject* go) {
	//	go->LetsGoHell();
	//}

private:

	std::list<GORegister> m_gameObjectList;
	//std::list< std::unique_ptr<IGameObject> > m_gameObjectList;

};

class GONewDeleteManager {

public:

	//GOを"つくるだけ"。AddGOが必要なものはAddGOして。
	template<class T, class... TArgs>
	T*  NewGO(TArgs... ctorArgs) {
		//newする+フラグおん
		T* newObject = new T(ctorArgs...);
		newObject->MarkNewGOMark();
		return newObject;
	}

	void DeleteGO(IGameObject* gameObject) {
		//殺す//殺しフラグ立てる
		if (!gameObject->GetNewGOMark()) {
			//Newgoのフラグ立ってなかったらエラー
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "NewGOされていないゲームオブジェクトをDeleteGOしようとしています。\n型名:%s", typeid(gameObject).name());
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
		}
		else {
			delete gameObject;
		}
	}

	//void FarewellDearDeadman() {
		//殺す
	//}
};

}