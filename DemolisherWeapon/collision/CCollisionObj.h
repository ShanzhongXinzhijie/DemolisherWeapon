#pragma once

#include "../physics/PhysicsBaseObject.h"
#include "../util/Util.h"

#define BIT(x) (1<<(x))

namespace DemolisherWeapon{

enum EnCollisionTimer {
	enNoTimer = -1,
};

/*enum CollisionFilterGroups
{
	CollisionObjFilter = BIT(6),//64,
};*/

namespace GameObj {
	class CCollisionObj;
}
struct RegColObj
{
	RegColObj(GameObj::CCollisionObj* p) : m_CObj(p) {}
	bool m_isEnable = true;
	GameObj::CCollisionObj* m_CObj = nullptr;
};

namespace GameObj{

class CCollisionObj : public PhysicsBaseObject , public IGameObject {

public:

	//コールバック関数の引数
	struct SCallbackParam {
		const int m_nameKey;
		void* m_voidPtr;
		const btGhostObject&  m_ghostObject;
		IDW_Class* m_classPtr;

		//名前が一致するか判定
		bool EqualName(const wchar_t* name) {
			return m_nameKey == Util::MakeHash(name);
		}

		//クラスを取り出す
		template<class T>
		T* GetClass() {
			T* p = dynamic_cast<T*>(m_classPtr);
			if (p == nullptr) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "クラスの取り出し(型変換)に失敗しました。\nSCallbackParam::GetClass()の型名を確認してください。\n変換先型名:%s\n変換元型名:%s", typeid(T).name(), typeid(m_classPtr).name());
				MessageBox(NULL, message, "Error", MB_OK);
				std::abort();
#endif
			}
			return p;
		};

		//ポインタを取り出す
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
		/*
#ifndef DW_MASTER
		if (m_isregistered) {
			MessageBox(NULL, "CCollisionObjが不正に削除されました。\nCCollisionObj::Delete()を使って削除してください。", "Error", MB_OK);
			std::abort();
		}
#endif
		*/
		if (m_isregistered) {
			m_register->m_isEnable = false;//登録無効化しとく
		}
		Release();
	};

public:
	void Release()override;

	//削除する
	void Delete() {
		//デリートレジストされてたら無効化だけ
		if (m_isregistered) {
			m_isDeath = true;
		}
		else {
			delete this; return;
		}
	}

	/*!
	* @brief	座標を設定。
	*/
	void SetPosition(const CVector3& pos)
	{
		auto& btTrans = m_ghostObject.getWorldTransform();
		btVector3 btPos;
		pos.CopyTo(btPos);
		btTrans.setOrigin(btPos);
	}
	/*!
	* @brief	回転を設定。
	*/
	void SetRotation(const CQuaternion& rot)
	{
		auto& btTrans = m_ghostObject.getWorldTransform();
		btQuaternion btRot;
		rot.CopyTo(btRot);
		btTrans.setRotation(btRot);
	}

	//名前を設定
	void SetName(const wchar_t* name) {
		m_nameKey = Util::MakeHash(name);
	}
	//コールバック関数を設定
	void SetCallback(std::function<void(SCallbackParam&)> callbackFunction) {
		m_callback = callbackFunction;
	}
	//クラスのポインタを設定
	void SetClass(IDW_Class* classPtr) {
		m_classPtr = classPtr;
	}
	//寿命を設定
	void SetTimer(int lefttimeFrame) {
		m_lifespan = max(lefttimeFrame, enNoTimer);
	}

	//ポインタを設定
	void SetPointer(void* pointer) {
		m_void = pointer;
	}

	//グループを設定
	void SetGroup(unsigned int group) {
		m_group = group;
	}
	//マスクを設定
	void SetMask(unsigned int mask) {
		m_mask = mask;
	}

	//グループの指定のビットをオンにする
	void On_OneGroup(unsigned int oneGroup) {
		m_group = m_group | BIT(oneGroup);
	}
	//マスクの指定のビットをオンにする
	void On_OneMask(unsigned int oneMask) {
		m_mask = m_mask | BIT(oneMask);
	}
	//グループの指定のビットをオフにする
	void Off_OneGroup(unsigned int oneGroup) {
		m_group = m_group & ~BIT(oneGroup);
	}
	//マスクの指定のビットをオフにする
	void Off_OneMask(unsigned int oneMask) {
		m_mask = m_mask & ~BIT(oneMask);
	}

	//すべてのグループに属するよう設定
	void All_On_Group() {
		m_group = 0xFFFFFFFF;
	}
	//すべてのグループと判定するようマスクを設定
	void All_On_Mask() {
		m_mask = 0xFFFFFFFF;
	}
	//どのグループにも属さないよう設定
	void All_Off_Group() {
		m_group = 0;
	}
	//どのグループとも判定しないようマスクを設定
	void All_Off_Mask() {
		m_mask = 0;
	}

	void PostUpdate()override {
		m_isregistered = false;
	}

	void Update() override{
		if (m_lifespan != enNoTimer && m_lifespan <= 0 || m_isDeath) { Delete(); }
		Register();
		if (m_lifespan != enNoTimer) { m_lifespan--; }
	}

	void RunCallback(SCallbackParam& param)
	{
		if (m_callback) {
			m_callback(param);
		}
	}

	//判定オブジェクトを取得
	btGhostObject& GetCollisionObject() { return m_ghostObject; }

	int GetNameKey()const { return m_nameKey; };
	void* GetData() { return m_void; };
	IDW_Class* GetClass() { return m_classPtr; };

	unsigned int GetGroup()const { return m_group; }
	unsigned int GetMask() const { return m_mask; }

	//判定してもいい状態か?
	bool IsEnable()const {
		return m_enable && m_isInit && !m_isDeath;
	}

private:
	
	/*!
	* @brief	ゴースト作成処理の共通処理。
	*/
	void CreateCommon(CVector3 pos, CQuaternion rot) override;

	//マネージャーに登録する
	void Register();

private:
	RegColObj* m_register = nullptr;

	bool m_enable = true;//有効?
	bool m_isDeath = false;//死?
	bool m_isInit = false;//初期化済み?
	bool m_isRegistPhysicsWorld = false;//!<物理ワールドに登録しているかどうかのフラグ。
	bool m_isregistered = false;//登録済み?

	int m_lifespan = 0;//寿命

	unsigned int m_group = 0;
	unsigned int m_mask = 0;// 0xFFFFFFFF;//すべて1

	int m_nameKey = 0;

	void* m_void = nullptr;
	IDW_Class* m_classPtr = nullptr;

	btGhostObject m_ghostObject;

	std::function<void(SCallbackParam&)> m_callback = nullptr;
};

class CollisionObjManager : public IGameObject{
public:

	//判定処理やる
	void PostUpdate()override final;

	RegColObj* AddCollisionObj(CCollisionObj* obj) {
		m_colObjList.emplace_back(obj);
		return &m_colObjList.back();
	};

private:

	std::list<RegColObj> m_colObjList;
};

}

}