#include "DWstdafx.h"
#include "CCollisionObj.h"

namespace DemolisherWeapon {

	void CCollisionObj::Release()
	{
		if (m_isRegistPhysicsWorld == true) {
			GetEngine().GetPhysicsWorld().RemoveCollisionObject(m_ghostObject);
			m_isRegistPhysicsWorld = false;
		}
	}

	void CCollisionObj::CreateCommon(CVector3 pos, CQuaternion rot)
	{
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);

		//�����G���W���ɓo�^�B
		//mask=0�ɂ��Ƃ�
#ifdef _DEBUG	
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter, 0);
		m_isRegistPhysicsWorld = true;
#endif

		m_isInit = true;
	}

	void CCollisionObj::Register() {
		if (!m_isregistered) {
			AddCollisionObj(this);
			m_isregistered = true;
		}
	}

	namespace {

		struct ObjManagerCallback : public btCollisionWorld::ContactResultCallback
		{
			CCollisionObj* ObjA = nullptr, *ObjB = nullptr;

			ObjManagerCallback(CCollisionObj* A, CCollisionObj* B) : ObjA(A), ObjB(B) {};

			virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
			{

				//�e�X�������s
				CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetData(), ObjB->Getbt() };
				ObjA->RunCallback(paramB);
				CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetData(), ObjA->Getbt() };
				ObjB->RunCallback(paramA);

				return 0.0f;
			};
		};

		bool Masking(unsigned int group, unsigned int mask){
			return (group & mask) != 0;
		}
	}

	void CollisionObjManager::PostUpdate() {
		//if(m_colObjList.size()>1){
		for (auto itr = m_colObjList.begin(); itr != m_colObjList.end(); ++itr) {

			CCollisionObj* ObjA = *itr;

			if (!ObjA->IsEnable()) { continue; }

			auto itr2 = itr; ++itr2;
			for (itr2; itr2 != m_colObjList.end(); ++itr2) {

				CCollisionObj* ObjB = *itr2;

				if (!ObjB->IsEnable()) { continue; }

				if (Masking(ObjA->GetGroup(),ObjB->GetMask()) && Masking(ObjB->GetGroup(), ObjA->GetMask())) {//�}�X�L���O
					ObjManagerCallback callback(ObjA, ObjB);
					GetPhysicsWorld().ContactPairTest(&ObjA->GetObj(), &ObjB->GetObj(), callback);
				}
			}
		}
		//}
		m_colObjList.clear();
	}

}