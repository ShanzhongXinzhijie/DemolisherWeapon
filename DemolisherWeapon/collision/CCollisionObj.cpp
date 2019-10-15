#include "DWstdafx.h"
#include "CCollisionObj.h"
#include "physics/CollisionAttr.h"

namespace DemolisherWeapon {

namespace GameObj {

namespace Suicider {

	void CCollisionObj::Release()
	{
		if (m_isRegistPhysicsWorld == true) {
			GetEngine().GetPhysicsWorld().RemoveCollisionObject(m_ghostObject);
			m_isRegistPhysicsWorld = false;
		}
	}

	void CCollisionObj::CreateCommon(const CVector3& pos, const CQuaternion& rot)
	{
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		m_ghostObject.setCollisionFlags(m_ghostObject.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		m_ghostObject.setUserIndex(enCollisionAttr_CCollisionObj);
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);
		m_btOldTrans = m_ghostObject.getWorldTransform();

		m_ghostObject.setUserPointer(this);

		//物理エンジンに登録
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter | CCollisionObjFilter, CCollisionObjFilter);// btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter ^ CCollisionObjFilter);
		m_isRegistPhysicsWorld = true;

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

		struct ObjManagerCallback : public btCollisionWorld::ContactResultCallback
		{
			RegColObj* RegiObjA = nullptr;

			ObjManagerCallback(RegColObj* A, bool isCollisionStaticObject) : RegiObjA(A) {
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
				
				//このループで既に実行した組み合わせか?
				if (ObjB->GetContactTestEnable() && RegiObjA->m_CObj->GetIndex() > ObjB->GetIndex()) {
					return false;
				}

				//マスク判定
				if (!(Masking(RegiObjA->m_CObj->GetGroup(), ObjB->GetMask()) && Masking(ObjB->GetGroup(), RegiObjA->m_CObj->GetMask()))) {
					return false;
				}

				return true;
			};

			//接触の数だけ実行される
			btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
			{
				Suicider::CCollisionObj* ObjA = nullptr; 
				bool old_hanteingA;

				if (colObj0Wrap->getCollisionObject()->getUserIndex() == enCollisionAttr_CCollisionObj) {
					ObjA = (Suicider::CCollisionObj*)(colObj0Wrap->getCollisionObject()->getUserPointer());
					old_hanteingA = ObjA->GetHanteing();
					ObjA->SetHanteing(true);
				}

				Suicider::CCollisionObj* ObjB = nullptr; 
				bool old_hanteingB;

				if (colObj1Wrap->getCollisionObject()->getUserIndex() == enCollisionAttr_CCollisionObj) {
					ObjB = (Suicider::CCollisionObj*)(colObj1Wrap->getCollisionObject()->getUserPointer());
					old_hanteingB = ObjB->GetHanteing();
					ObjB->SetHanteing(true);
				}
				
				if (ObjA && ObjB) {
					//各々処理実行
					Suicider::CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetName(), ObjB->GetPointer(), ObjB->GetCollisionObject(), ObjB->GetClass(), false, &cp, true };
					ObjA->RunCallback(paramB);
					Suicider::CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetName(), ObjA->GetPointer(), ObjA->GetCollisionObject(), ObjA->GetClass(), true, &cp, true };
					ObjB->RunCallback(paramA);
				}
				else {
					if (ObjA) {
						Suicider::CCollisionObj::SCallbackParam param = { -1, nullptr, nullptr, dummy, nullptr, false, &cp, false };
						ObjA->RunCallback(param);
					}
					else if (ObjB) {
						Suicider::CCollisionObj::SCallbackParam param = { -1, nullptr, nullptr, dummy, nullptr, true, &cp, false };
						ObjB->RunCallback(param);
					}
				}
				
				if (ObjA) { ObjA->SetHanteing(old_hanteingA); }
				if (ObjB) { ObjB->SetHanteing(old_hanteingB); }

				return 0.0f;
			};
		};
		struct ObjManagerConvexCallback : public btCollisionWorld::ConvexResultCallback
		{
			RegColObj* RegiObjA = nullptr;

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
				//削除されてないか?
				if (proxy0 == nullptr || !RegiObjA->m_isEnable) {
					return false;
				}

				//Bulletのマスク判定
				bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
				collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
				if (!collides) { return false; }

				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)((btCollisionObject*)proxy0->m_clientObject)->getUserPointer();

				//自分でない
				if (RegiObjA->m_CObj == ObjB) {
					return false;
				}

				//登録無効化されてないか?
				if (!ObjB->IsEnable() || !RegiObjA->m_CObj->IsEnable()) {
					return false;
				}

				//このループで既に実行した組み合わせか?
				if (ObjB->GetContactTestEnable() && RegiObjA->m_CObj->GetIndex() > ObjB->GetIndex()) {
					return false;
				}

				//マスク判定
				if (!(Masking(RegiObjA->m_CObj->GetGroup(), ObjB->GetMask()) && Masking(ObjB->GetGroup(), RegiObjA->m_CObj->GetMask()))) {
					return false;
				}

				return true;
			};

			//接触の数だけ実行される
			btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)override
			{				
				Suicider::CCollisionObj* ObjA = RegiObjA->m_CObj;
				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)(convexResult.m_hitCollisionObject->getUserPointer());

				bool old_hanteingA = ObjA->GetHanteing();
				bool old_hanteingB = ObjB->GetHanteing();

				ObjA->SetHanteing(true);
				ObjB->SetHanteing(true);

				//各々処理実行
				Suicider::CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetName(), ObjB->GetPointer(), ObjB->GetCollisionObject(), ObjB->GetClass(), false };
				ObjA->RunCallback(paramB);
				Suicider::CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetName(), ObjA->GetPointer(), ObjA->GetCollisionObject(), ObjA->GetClass(), true };
				ObjB->RunCallback(paramA);

				ObjA->SetHanteing(old_hanteingA);
				ObjB->SetHanteing(old_hanteingB);

				return 0.0f;
			};
		};
	}

	void CollisionObjManager::PostUpdate() {
		for (auto itr = m_colObjList.begin(); itr != m_colObjList.end(); ++itr) {

			Suicider::CCollisionObj* ObjA = (*itr).m_CObj;

			if (!(*itr).m_isEnable || !ObjA->IsEnable() || !ObjA->GetContactTestEnable() ) { continue; }

			ObjA->SetHanteing(true);//判定中!

			ObjManagerCallback callback(&(*itr), ObjA->GetIsCollisionStaticObject());
			GetPhysicsWorld().ContactTest(&ObjA->GetCollisionObject(), callback);
			//ObjManagerConvexCallback callback(&(*itr));
			//GetPhysicsWorld().ConvexSweepTest((const btConvexShape*)ObjA->GetCollisionObject().getCollisionShape(), ObjA->GetBtOldTrans(), ObjA->GetCollisionObject().getWorldTransform(), callback);

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