#pragma once

#include <bitset>
#include "physics/PhysicsBaseObject.h"

namespace DemolisherWeapon{

enum EnCollisionTimer {
	enNoTimer = -1,
};

inline constexpr int CCollisionObjFilter = 64;

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
	//コールバック関数の引数
	struct SCallbackParam {
		//あたった相手の情報
		const int m_nameKey;
		const wchar_t* m_name;
		void* m_voidPtr;
		const btGhostObject&  m_ghostObject;
		IDW_Class* m_classPtr;		
		const bool m_isA;//あたった相手がAか?
		bool m_isCCollisionObj = true;//これはCCollisionObjか?

		CVector3 m_collisionPoint;//衝突位置(かなり雑かも)

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
	};	

public:
	//コンストラクタ
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

	//デストラクタ
	~CCollisionObj() {
#ifndef DW_MASTER
		if (m_isHanteing) {
			//判定中
			MessageBox(NULL, "判定中のCCollisionObjが削除されました。\nやめなさい。\nCCollisionObj::DeleteやDeleteGOを使えばいいんじゃないかな", "Error", MB_OK);
			std::abort();
		}
#endif

		if (m_isregistered) {
			m_register->m_isEnable = false;//登録無効化しとく
		}
		Release();
	};

	//マネージャーに未登録フラグを設定(CollisionObjManager用)
	//※ユーザーは使用しないでください
	void NonReg() {
		m_isregistered = false;
		m_register = nullptr;
	}

public:
	//削除する
	void Delete() {
		m_isDeath = true;
	}
	//次のループで削除する(次のループでは判定されません)
	void DeleteNextLoop() {
		m_killMark = true;
	}

	//有効・無効の設定
	void SetEnable(bool enable){
		m_enable = enable;
	}
	bool GetEnable()const {
		return m_enable;
	}

	//自分から判定するか
	void SetContactTestEnable(bool enable) {
		m_contactTestEnable = enable;
	}
	bool GetContactTestEnable()const {
		return m_contactTestEnable;
	}

	//喰らい判定かどうか設定
	void SetIsHurtCollision(bool isHurtCol) {
		SetContactTestEnable(!isHurtCol);
	}

	//高速で動く判定かどうか設定
	void SetIsHighSpeed(bool isHighSpeed) {
		m_isHighSpeed = isHighSpeed;
	}
	bool GetIsHighSpeed()const {
		return m_isHighSpeed;
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
		m_name = name;
	}
	const wchar_t* GetName()const {
		return m_name.c_str();
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
	int GetTimer()const {
		return m_lifespan;
	}

	//ポインタを設定
	void SetPointer(void* pointer) {
		m_void = pointer;
	}

	//判定ビットマスク
	typedef std::bitset<sizeof(unsigned int)*CHAR_BIT> Bitset;

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
		m_group.set(oneGroup);
	}
	//マスクの指定のビットをオンにする
	void On_OneMask(unsigned int oneMask) {
		m_mask.set(oneMask);
	}
	//グループの指定のビットをオフにする
	void Off_OneGroup(unsigned int oneGroup) {
		m_group.reset(oneGroup);
	}
	//マスクの指定のビットをオフにする
	void Off_OneMask(unsigned int oneMask) {
		m_mask.reset(oneMask);
	}

	//すべてのグループに属するよう設定
	void All_On_Group() {
		m_group = 0; m_group.flip();
	}
	//すべてのグループと判定するようマスクを設定
	void All_On_Mask() {
		m_mask = 0; m_mask.flip();
	}
	//どのグループにも属さないよう設定
	void All_Off_Group() {
		m_group = 0;
	}
	//どのグループとも判定しないようマスクを設定
	void All_Off_Mask() {
		m_mask = 0;
	}

	//静的オブジェクトと判定するか設定
	void SetIsCollisionStaticObject(bool isCollision) {
		m_isCollisionStaticObject = isCollision;
	}
	//静的オブジェクトと判定するか取得
	bool GetIsCollisionStaticObject()const {
		return m_isCollisionStaticObject; 
	}

	//このあたり判定を静的オブジェクトとしても初期化するか設定
	//※当たり判定形状を作成する前に設定してください
	void SetIsStaticObject(bool isStatic) {
		m_isStaticObj = isStatic;
	}

	void PostUpdate()override {
		//実行順がCollisionObjManagerより遅いからできる処理
		m_isregistered = false;
		m_register = nullptr;
		m_btOldTrans = m_ghostObject.getWorldTransform();
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

	//判定オブジェクトを取得
	btGhostObject& GetCollisionObject() { return m_ghostObject; }
	const btTransform& GetBtOldTrans() { return m_btOldTrans; }

	int GetNameKey()const { return m_nameKey; };
	void* GetPointer() { return m_void; };
	IDW_Class* GetClass() { return m_classPtr; };

	//判定グループ・マスク取得
	unsigned long GetGroup()const { return m_group.to_ulong(); }
	unsigned long GetMask() const { return m_mask.to_ulong(); }
	const Bitset& GetGroupBitset()const { return m_group; }
	const Bitset& GetMaskBitset() const { return m_mask; }

	//判定してもいい状態か?
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

	//判定中かのフラグ
	void SetHanteing(bool flag) {
		m_isHanteing = flag;
	}
	bool GetHanteing()const {
		return m_isHanteing;
	}

private:
	//開放処理
	void Release()override;

	/*!
	* @brief	ゴースト作成処理の共通処理。
	*/
	void CreateCommon(const CVector3& pos, const CQuaternion& rot) override;

	//マネージャーに登録する
	void Register(bool compulsion = false);

private:
	RegColObj* m_register = nullptr;

	bool m_enable = true;//有効?
	bool m_contactTestEnable = true;//自分から判定する?
	bool m_isDeath = false;//死?
	bool m_killMark = false;//死期が近い?
	bool m_isInit = false;//初期化済み?
	bool m_isRegistPhysicsWorld = false;//!<物理ワールドに登録しているかどうかのフラグ。
	bool m_isregistered = false;//登録済み?

	bool m_isHanteing = false; //判定中か?
	
	int m_lifespan = 0;//寿命

	bool m_isCollisionStaticObject = false; //静的オブジェクトと判定するか
	bool m_isHighSpeed = false; //動くスピードが速いか?

	//判定ビットマスク
	Bitset m_group;
	Bitset m_mask;

	//名前
	int m_nameKey = 0;
	std::wstring m_name;

	void* m_void = nullptr;
	IDW_Class* m_classPtr = nullptr;

	btGhostObject m_ghostObject;
	btTransform m_btOldTrans;
	bool m_isStaticObj = false;//静的オブジェクトでもあるか?

	std::function<void(SCallbackParam&)> m_callback = nullptr;
};

}

class CollisionObjManager : public IGameObject{
public:
	//判定処理やる
	void PostUpdate()override final;

	//コリジョンを登録
	RegColObj* AddCollisionObj(Suicider::CCollisionObj* obj) {
		m_colObjList.emplace_back(obj, (int)m_colObjList.size());
		return &m_colObjList.back();
	};

private:
	std::list<RegColObj> m_colObjList;
};

}

//CCollisionObjに対するレイ判定用(Closest)
struct ClosestRayResultCallbackForCCollisionObj : public btCollisionWorld::ClosestRayResultCallback
{
	//コンストラクタ
	ClosestRayResultCallbackForCCollisionObj(const btVector3& rayFromWorld, const btVector3& rayToWorld)
		: ClosestRayResultCallback(rayFromWorld, rayToWorld)
	{
		m_collisionFilterMask = CCollisionObjFilter;//CCollisionObjとのみ判定
	}
	ClosestRayResultCallbackForCCollisionObj(const btVector3& rayFromWorld, const btVector3& rayToWorld, const wchar_t* name)
		: ClosestRayResultCallback(rayFromWorld, rayToWorld)
	{
		m_collisionFilterMask = CCollisionObjFilter;//CCollisionObjとのみ判定

		SetTargetCollisionName(name);//ターゲットの名前を設定
	}

	//衝突判定を取るかどうかの判定
	bool needsCollision(btBroadphaseProxy* proxy0) const override
	{
		bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
		collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		if (collides && useNameKey) {
			//CCollisionObjをとりだす
			btCollisionObject* btObj = reinterpret_cast<btCollisionObject*>(proxy0->m_clientObject);
			GameObj::Suicider::CCollisionObj* colObj = reinterpret_cast<GameObj::Suicider::CCollisionObj*>(btObj->getUserPointer());

			//名前が一致するか判定
			collides = (colObj->GetNameKey() == collisionNameKey);
		}

		return collides;
	}

	//衝突対象となる名前を設定する
	void SetTargetCollisionName(const wchar_t* name) {
		useNameKey = true;
		collisionNameKey = Util::MakeHash(name);
	}

private:
	bool useNameKey = false;
	int collisionNameKey = 0;
};

//CCollisionObjに対するレイ判定用(AllHits)
struct AllHitsRayResultCallbackForCCollisionObj : public btCollisionWorld::AllHitsRayResultCallback
{
	//コンストラクタ
	AllHitsRayResultCallbackForCCollisionObj(const btVector3& rayFromWorld, const btVector3& rayToWorld)
		: AllHitsRayResultCallback(rayFromWorld, rayToWorld)
	{
		m_collisionFilterMask = CCollisionObjFilter;//CCollisionObjとのみ判定
	}
	AllHitsRayResultCallbackForCCollisionObj(const btVector3& rayFromWorld, const btVector3& rayToWorld, const wchar_t* name)
		: AllHitsRayResultCallback(rayFromWorld, rayToWorld)
	{
		m_collisionFilterMask = CCollisionObjFilter;//CCollisionObjとのみ判定

		SetTargetCollisionName(name);//ターゲットの名前を設定
	}

	//衝突判定を取るかどうかの判定
	bool needsCollision(btBroadphaseProxy* proxy0) const override
	{
		bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
		collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		if (collides && useNameKey) {
			//CCollisionObjをとりだす
			btCollisionObject* btObj = reinterpret_cast<btCollisionObject*>(proxy0->m_clientObject);
			GameObj::Suicider::CCollisionObj* colObj = reinterpret_cast<GameObj::Suicider::CCollisionObj*>(btObj->getUserPointer());

			//名前が一致するか判定
			collides = (colObj->GetNameKey() == collisionNameKey);
		}

		return collides;
	}

	//衝突対象となる名前を設定する
	void SetTargetCollisionName(const wchar_t* name) {
		useNameKey = true;
		collisionNameKey = Util::MakeHash(name);
	}

private:
	bool useNameKey = false;
	int collisionNameKey = 0;
};

}