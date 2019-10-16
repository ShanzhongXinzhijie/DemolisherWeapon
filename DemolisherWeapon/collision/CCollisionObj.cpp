#include "DWstdafx.h"
#include "CCollisionObj.h"
#include "physics/CollisionAttr.h"

namespace DemolisherWeapon {

namespace GameObj {

namespace Suicider {

	void CCollisionObj::Release()
	{
		//物理エンジンから削除
		if (m_isRegistPhysicsWorld == true) {
			GetEngine().GetPhysicsWorld().RemoveCollisionObject(m_ghostObject);
			m_isRegistPhysicsWorld = false;
		}
	}

	void CCollisionObj::CreateCommon(const CVector3& pos, const CQuaternion& rot)
	{
		//ゴーストオブジェクトの設定
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		m_ghostObject.setCollisionFlags(m_ghostObject.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		m_ghostObject.setUserIndex(enCollisionAttr_CCollisionObj);
		m_ghostObject.setUserPointer(this);
		
		//トランスフォームの設定
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);
		m_btOldTrans = m_ghostObject.getWorldTransform();
		
		//衝突マスク
		short int mask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter;
		if (!m_isStaticObj) {
			mask = CCollisionObjFilter;
		}

		//物理エンジンに登録
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter | CCollisionObjFilter, mask);
		m_isRegistPhysicsWorld = true;

		//初期化完了
		m_isInit = true;
	}

	void CCollisionObj::Register(bool compulsion) {
		if (!m_isregistered && (compulsion || IsEnable()) && GetContactTestEnable()) {
			m_register = AddCollisionObj(this);
			m_isregistered = true;
		}
	}

}

	namespace {

		bool Masking(unsigned int group, unsigned int mask) {
			return (group & mask) != 0;
		}

		const btGhostObject dummy;//ダミー用

		bool NeedsCollision(btBroadphaseProxy* proxy0, RegColObj* RegiObjA, short int	m_collisionFilterGroup, short int	m_collisionFilterMask) {
			//削除されてないか?
			if (proxy0 == nullptr || !RegiObjA->m_isEnable) {
				return false;
			}

			//Bulletのマスク判定
			bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
			collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			if (!collides) { return false; }

			//CCollisionObjでなければここでtrue
			if (!(proxy0->m_collisionFilterGroup & CCollisionObjFilter)) {
				return true;
			}

			Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)((btCollisionObject*)proxy0->m_clientObject)->getUserPointer();

			//登録無効化されてないか?
			if (!ObjB->IsEnable() || !RegiObjA->m_CObj->IsEnable()) {
				return false;
			}

			//Aがハイスピードじゃなくて…
			if (!RegiObjA->m_CObj->GetIsHighSpeed()) {
				//このループで既に実行した組み合わせか?
				//Bがくらい判定じゃなくて過去に処理実行してたらreturn
				if (ObjB->GetContactTestEnable() && RegiObjA->m_CObj->GetIndex() > ObjB->GetIndex()) {
					return false;
				}
			}

			//マスク判定
			if (!(Masking(RegiObjA->m_CObj->GetGroup(), ObjB->GetMask()) && Masking(ObjB->GetGroup(), RegiObjA->m_CObj->GetMask()))) {
				return false;
			}

			return true;
		}

		void AddSingleResult(Suicider::CCollisionObj* ObjA, Suicider::CCollisionObj* ObjB, const CVector3& hitPos){
			bool old_hanteingA;
			bool old_hanteingB;

			if (ObjA) {
				old_hanteingA = ObjA->GetHanteing();
				ObjA->SetHanteing(true);
			}
			if (ObjB) {
				old_hanteingB = ObjB->GetHanteing();
				ObjB->SetHanteing(true);
			}

			if (ObjA && ObjB) {
				//各々処理実行
				Suicider::CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetName(), ObjB->GetPointer(), ObjB->GetCollisionObject(), ObjB->GetClass(), false, true, hitPos };
				ObjA->RunCallback(paramB);
				Suicider::CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetName(), ObjA->GetPointer(), ObjA->GetCollisionObject(), ObjA->GetClass(), true, true, hitPos };
				ObjB->RunCallback(paramA);
			}
			else {
				if (ObjA) {
					Suicider::CCollisionObj::SCallbackParam param = { -1, nullptr, nullptr, dummy, nullptr, false, false, hitPos };
					ObjA->RunCallback(param);
				}
				else if (ObjB) {
					Suicider::CCollisionObj::SCallbackParam param = { -1, nullptr, nullptr, dummy, nullptr, true, false, hitPos };
					ObjB->RunCallback(param);
				}
			}

			if (ObjA) { ObjA->SetHanteing(old_hanteingA); }
			if (ObjB) { ObjB->SetHanteing(old_hanteingB); }
		}

		struct ObjManagerCallback : public btCollisionWorld::ContactResultCallback
		{
			RegColObj* RegiObjA = nullptr;

			//コンストラクタ
			ObjManagerCallback(RegColObj* A, bool isCollisionStaticObject) : RegiObjA(A) {
				if (isCollisionStaticObject) {
					m_collisionFilterMask = CCollisionObjFilter | btBroadphaseProxy::StaticFilter;//CCollisionObjと静的オブジェクトのみ判定
				}
				else {
					m_collisionFilterMask = CCollisionObjFilter;//CCollisionObjとのみ判定
				}
				m_collisionFilterGroup = btBroadphaseProxy::StaticFilter | CCollisionObjFilter;
			}

			//判定するかどうか判定する
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				return NeedsCollision(proxy0, RegiObjA, m_collisionFilterGroup, m_collisionFilterMask);
			}

			//接触の数だけ実行される
			btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
			{
				Suicider::CCollisionObj* ObjA = nullptr; 
				if (colObj0Wrap->getCollisionObject()->getUserIndex() == enCollisionAttr_CCollisionObj) {
					ObjA = (Suicider::CCollisionObj*)(colObj0Wrap->getCollisionObject()->getUserPointer());
				}
				Suicider::CCollisionObj* ObjB = nullptr; 
				if (colObj1Wrap->getCollisionObject()->getUserIndex() == enCollisionAttr_CCollisionObj) {
					ObjB = (Suicider::CCollisionObj*)(colObj1Wrap->getCollisionObject()->getUserPointer());
				}
				
				AddSingleResult(ObjA, ObjB, (cp.getPositionWorldOnA()+cp.getPositionWorldOnB())/2.0f);

				return 0.0f;
			}
		};
		struct ObjManagerConvexCallback : public btCollisionWorld::ConvexResultCallback
		{
			RegColObj* RegiObjA = nullptr;

			//コンストラクタ
			ObjManagerConvexCallback(RegColObj* A, bool isCollisionStaticObject) : RegiObjA(A) {
				if (isCollisionStaticObject) {
					m_collisionFilterMask = CCollisionObjFilter | btBroadphaseProxy::StaticFilter;//CCollisionObjと静的オブジェクトのみ判定
				}
				else {
					m_collisionFilterMask = CCollisionObjFilter;//CCollisionObjとのみ判定
				}
				m_collisionFilterGroup = btBroadphaseProxy::StaticFilter | CCollisionObjFilter;
			};

			//判定するかどうか判定する
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				//じぶんでない
				if ((btCollisionObject*)proxy0->m_clientObject == (btCollisionObject*)&(RegiObjA->m_CObj->GetCollisionObject())) {
					return false;
				}
				return NeedsCollision(proxy0, RegiObjA, m_collisionFilterGroup, m_collisionFilterMask);
			};

			//接触の数だけ実行される
			btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)override
			{				
				Suicider::CCollisionObj* ObjA = nullptr; ObjA = RegiObjA->m_CObj;
				Suicider::CCollisionObj* ObjB = nullptr; 
				if (convexResult.m_hitCollisionObject->getUserIndex() == enCollisionAttr_CCollisionObj) {
					ObjB = (Suicider::CCollisionObj*)(convexResult.m_hitCollisionObject->getUserPointer());
				}

				AddSingleResult(ObjA, ObjB, convexResult.m_hitPointLocal);

				return 0.0f;
			};
		};
	}

	void CollisionObjManager::PostUpdate() {
		for (auto itr = m_colObjList.begin(); itr != m_colObjList.end(); ++itr) {

			Suicider::CCollisionObj* ObjA = (*itr).m_CObj;

			if (!(*itr).m_isEnable || !ObjA->IsEnable() || !ObjA->GetContactTestEnable() ) { continue; }

			ObjA->SetHanteing(true);//判定中!

			if (ObjA->GetIsHighSpeed()) {
				ObjManagerConvexCallback callback(&(*itr), ObjA->GetIsCollisionStaticObject());
				GetPhysicsWorld().ConvexSweepTest((const btConvexShape*)ObjA->GetCollisionObject().getCollisionShape(), ObjA->GetBtOldTrans(), ObjA->GetCollisionObject().getWorldTransform(), callback);
			}
			else {
				ObjManagerCallback callback(&(*itr), ObjA->GetIsCollisionStaticObject());
				GetPhysicsWorld().ContactTest(&ObjA->GetCollisionObject(), callback);
			}
			
			ObjA->SetHanteing(false);//判定終わり
		}
		for (auto& reg : m_colObjList) {
			if (reg.m_isEnable) {
				reg.m_CObj->NonReg();
			}
		}
		m_colObjList.clear();
	}

}

}