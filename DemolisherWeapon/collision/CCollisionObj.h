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

namespace GameObj{

class CCollisionObj : public PhysicsBaseObject , public IGameObject {

public:

	//コールバック関数の引数
	struct SCallbackParam {
		const int m_nameKey;
		void* m_class;
		const btGhostObject&  m_ghostObject;

		//名前が一致するか判定
		bool EqualName(const wchar_t* name) {
			return m_nameKey == Util::MakeHash(name);
		}

		//ポインタを取り出す
		template<class T>
		T* GetGO() {
			return (T*)m_class;
		};
	};	

	/*static CCollisionObj* New(int lifespan = 1) {
		return new CCollisionObj(lifespan);
	}

private:*/
	CCollisionObj(int lifespan = enNoTimer, const wchar_t* name = nullptr, void* pointer = nullptr, std::function<void(SCallbackParam&)> callbackFunction = nullptr) {
		m_lifespan = max(lifespan, enNoTimer);
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
	//ポインタを設定
	void SetPointer(void* pointer) {
		m_void = pointer;
	}
	//寿命を設定
	void SetTimer(int lefttime) {
		m_lifespan = max(lefttime, enNoTimer);
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

	btGhostObject m_ghostObject;

	std::function<void(SCallbackParam&)> m_callback = nullptr;
};

class CollisionObjManager : public IGameObject{
public:

	//判定処理やる
	void PostUpdate()override final;

	void AddCollisionObj(CCollisionObj* obj) {
		m_colObjList.push_back(obj);
	};

private:

	std::list<CCollisionObj*> m_colObjList;
};

}

}