#pragma once

#include <unordered_map>

#include "../util/Util.h"

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
//ステータスレシーバー
class GOStatusReceiver {
public:
	~GOStatusReceiver() {
		if(m_registerCaster) m_registerCaster->ImDead();
	}

	//なんか初期設定
	//ユーザーは使わないでください
	void SetCaster(GOStatusCaster* caster) {
		m_registerCaster = caster;
	}
	//ユーザーは使わないでください
	void SetGameObject(IGameObject* go) {
		m_ptrGO = go;
	}

	//ステータス更新
	//ユーザーは使わないでください
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

class GODeathListener;
//死亡リスナー登録クラス
struct GODeathListenerRegister {
	GODeathListenerRegister(GODeathListener* listener_ptr) {
		listener = listener_ptr;
	}
	~GODeathListenerRegister();

	bool enable = true;
	GODeathListener* listener = nullptr;
};
//死亡リスナー
class GODeathListener {
public:
	~GODeathListener() {
		if (m_resister) { m_resister->enable = false; }
	}

	//デスリスナーの引数
	struct SDeathParam {
		IGameObject* gameObject = nullptr;
	};	

	//死亡通知時に実行される関数を設定
	void SetFunction(std::function<void(const SDeathParam& param)> func) {
		m_function = func;
	}

	//ユーザーは使わないでください
	void SetResister(GODeathListenerRegister* resister) {
		m_resister = resister;
	}
	//ユーザーは使わないでください
	void RunFunction(const SDeathParam& param) {
		m_function(param);
	}

private:
	std::function<void(const SDeathParam& param)> m_function;
	GODeathListenerRegister* m_resister = nullptr;
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
	IGameObject(bool isRegister = true, bool quickStart = false);
	virtual ~IGameObject() {
		//有効でないんだ！
		if (IsRegistered()) { m_register->isEnable = false; }
		//ステータス更新(死んだぞ！)
		m_status.m_isDead = true;
		CastStatus();
		//デスリスナーに通知
		GODeathListener::SDeathParam param;
		param.gameObject = this;
		auto it = m_deathListeners.begin();
		while (it != m_deathListeners.end()) {
			if ((*it).enable) {
				(*it).listener->RunFunction(param);
				it++;
			}
			else {
				it = m_deathListeners.erase(it);//削除
			}
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
	//DeleteGOされた
	void お前はもう死んでいる() {
		m_isDead = true;
	}

public:
	//NewGOで作ったか?
	bool GetNewGOMark() const {
		return m_newgoMark;
	}
	//DeleteGOされてる?
	bool お前はもう死んでいる？() const{
		return m_isDead;
	}

	//有効化
	void SetEnable(bool e){
		m_enable = e;
	}
	//有効なのか？
	bool GetEnable() const{
		//if (m_goToHell) { return false; }
		return m_enable && !m_isDead;
	}

	//開始しているのか？
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
	//ステータスレシーバー登録解除
	void RemoveStatusReceiver(GOStatusReceiver* receiver) {
		for (auto& C : m_statusCaster) {
			if (C.GetReceiver() == receiver) {
				C.ImDead();
			}
		}
	}

	//死亡リスナー登録
	void AddDeathListener(GODeathListener* listener)
	{
		m_deathListeners.emplace_back(listener);
		listener->SetResister(&m_deathListeners.back());
	}
	//死亡リスナー登録解除
	void RemoveDeathListener(GODeathListener* listener) {
		for (auto& R : m_deathListeners) {
			if (R.listener == listener) {
				R.enable = false;
			}
		}
	}

	//名前をつける
	void SetName(const wchar_t* objectName);

public:
	//仮想関数

	//処理開始時に実行
	//戻り値がfalseだと処理開始しない
	virtual bool Start() { return true; };

	//ゲームループ前に実行
	virtual void PreLoopUpdate() {};
	
	//ゲームループ内で実行
	virtual void PreUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};

	//ゲームループ後に実行
	virtual void PostLoopUpdate() {};
	virtual void PostLoopPostUpdate() {};

	//3D描画前に実行(画面ごと)
	//int num 実行中の画面番号
	virtual void Pre3DRender(int num) {};

	//この関数内でHUDに2Dグラフィックを描画
	//int HUDNum 描画対象のHUDの番号
	//TODO HUDのレンダーターゲット作る
	virtual void HUDRender(int HUDNum) {};

	//2Dグラフィックをこの関数内で描画してください
	//※CFont,CSpriteなど
	virtual void PostRender() {};

private:
	bool m_isDead = false;//実質死亡
	bool m_enable = true;
	bool m_isStart = false;
	bool m_quickStart = false;

	GORegister* m_register = nullptr;//マネージャーに登録されているか(ポインタ)
	
	GOStatus m_status;//状態
	std::list<GOStatusCaster> m_statusCaster;//状態を送信する
	
	std::list<GODeathListenerRegister> m_deathListeners;//死亡リスナーさん達

	bool m_newgoMark = false;//NewGOで作ったか?

//　GameObjectManagerから操作できる	
	friend GameObjectManager;
	friend GONewDeleteManager;
};

//自動で登録をしないゲームオブジェクト
class INRGameObject : public IGameObject{
public:
	INRGameObject(bool quickStart = false) : IGameObject(false, quickStart) {};
	virtual ~INRGameObject() {};
};

//できる限り早く動作を開始するゲームオブジェクト
class IQSGameObject : public IGameObject {
public:
	IQSGameObject(bool isRegister = true) : IGameObject(isRegister, true) {};
	virtual ~IQSGameObject() {};
};

class GameObjectManager {
public:
	~GameObjectManager() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable) { go.gameObject->RegisterRegister(nullptr); }
		}
	}

	void Start() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && !go.gameObject->GetIsStart()) {
				if (go.gameObject->Start()) {
					go.gameObject->SetIsStart();
				}
			}
		}
	}
	void PreLoopUpdate() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PreLoopUpdate();
			}
		}
	}
	void Update() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PreUpdate();
			}
		}
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->Update();
			}
		}
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PostUpdate();
			}
		}
	}
	void PostLoopUpdate() {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PostLoopUpdate();
			}
		}
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PostLoopPostUpdate();
			}
		}		
	}
	void Pre3DRender(int num) {
		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->Pre3DRender(num);
			}
		}
	}
	void HUDRender(int HUDNum);
	void PostRender();

	//死の処理
	void Hell() {

		//m_gameObjectMapの削除
		{
			auto it = m_gameObjectMap.begin();
			while (it != m_gameObjectMap.end()) {
				if (!(*it).second->isEnable) {
					it = m_gameObjectMap.erase(it);//削除
				}
				else {
					++it;
				}
			}
		}

		//m_gameObjectListの削除
		{
			auto it = m_gameObjectList.begin();
			while (it != m_gameObjectList.end()) {
				if (!(*it).isEnable) {
					if ((*it).GetNowOnHell()) {//二回目で削除
						it = m_gameObjectList.erase(it);//削除
					}
					else {
						(*it).ArriveHell();
						++it;
					}
				}
				else {
					++it;
				}
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

	//ゲームオブジェクトに名前をつける
	void SetNameGO(IGameObject* go, const wchar_t* objectName) {
		if (!go) { return; }

		GORegister* regiGo = go->GetRegister();
		if (regiGo) {
			int nameKey = Util::MakeHash(objectName);			
			m_gameObjectMap.emplace(nameKey, regiGo);
		}
		else {
#ifndef DW_MASTER
			MessageBox(NULL, "登録されていないゲームオブジェクトに名前をつけようとしています", "Error", MB_OK);
			std::abort();
#endif
		}
	}

	//ゲームオブジェクトの検索
	template<class T>
	T* FindGO(const wchar_t* objectName)
	{
		auto range = m_gameObjectMap.equal_range(Util::MakeHash(objectName));

		for (auto it = range.first; it != range.second; ++it) {
			auto& regiGo = it->second;

			//有効か？
			if (regiGo->isEnable && regiGo->gameObject->GetEnable()) {
				//見つけた。
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
			//有効か？
			if (regiGo.isEnable && regiGo.gameObject->GetEnable()) {
				//見つけた。
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

			//有効か？
			if (regiGo->isEnable && regiGo->gameObject->GetEnable()) {
				//見つけた。
				T* p = dynamic_cast<T*>(regiGo->gameObject);
				if (p != nullptr) {
					if (func(p) == false) {
						//クエリ中断。
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
			//有効か？
			if (regiGo.isEnable && regiGo.gameObject->GetEnable()) {
				//見つけた。
				T* p = dynamic_cast<T*>(regiGo.gameObject);
				if (p != nullptr) {
					if (func(p) == false) {
						//クエリ中断。
						return;
					}
				}
			}
		}
	}

private:

	std::list<GORegister> m_gameObjectList;

	std::unordered_multimap<int, GORegister*> m_gameObjectMap;
};

class GONewDeleteManager {

public:

	template<class T, class... TArgs>
	T*  NewGO(TArgs... ctorArgs) {
		//newする+フラグおん
		T* newObject = new T(ctorArgs...);
		newObject->MarkNewGOMark();
		return newObject;
	}

	//(ゲームオブジェクトの無効化フラグが立つ。実際にインスタンスが削除されるのは、全てのGOのPostUpdateが終わってから)
	bool DeleteGO(IGameObject* gameObject, bool newgoCheck, bool instantKill = false) {
		
		if (!gameObject) { return false; }

		if (newgoCheck && !gameObject->GetNewGOMark()) {
			//Newgoのフラグ立ってなかったらエラー
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "NewGOされていないゲームオブジェクトをDeleteGOしようとしています。\n型名:%s", typeid(gameObject).name());
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
		}
		else {
			if (!gameObject->お前はもう死んでいる？()) {//まだ殺されていない
				//即座に殺す
				if (instantKill) {
					delete gameObject;
					return true;
				}
				
				//あとで殺す
				//無効化
				gameObject->お前はもう死んでいる();
				//殺しリスト登録
				m_deleteList.emplace_back(gameObject);
				return true;
			}
		}

		return false;
	}

	void FarewellDearDeadman() {
		//殺す
		for (auto& GO : m_deleteList) {
			delete GO;
		}
		m_deleteList.clear();
	}

private:
	std::list<IGameObject*> m_deleteList;

};

}