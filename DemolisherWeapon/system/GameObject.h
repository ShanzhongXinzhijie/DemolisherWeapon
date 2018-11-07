#pragma once

namespace DemolisherWeapon {

class IGameObject;
class GameObjectManager;
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

//ゲームオブジェクト
class IGameObject
{
public:
	IGameObject(bool isRegister = true);
	virtual ~IGameObject() {
		//有効でないんだ！
		if (IsRegistered()) { m_register->isEnable = false; }
		//ステータス更新(死んだぞ！)
		m_status.m_isDead = true;
		CastStatus();
	};

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

	//bool m_goToHell = false;//地獄に向かっているか?
	//bool m_nowOnHell = false;//地獄にいるか?

	GORegister* m_register = nullptr;//マネージャーに登録されているか(ポインタ)
	
	GOStatus m_status;//状態
	std::list<GOStatusCaster> m_statusCaster;//状態を送信する


//　GameObjectManagerから操作できる	
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

}