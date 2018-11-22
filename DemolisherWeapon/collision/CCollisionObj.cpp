#include "DWstdafx.h"
#include "CCollisionObj.h"

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

	void CCollisionObj::CreateCommon(CVector3 pos, CQuaternion rot)
	{
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		m_ghostObject.setCollisionFlags(m_ghostObject.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);

		m_ghostObject.setUserPointer(this);

		//�����G���W���ɓo�^
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter + CCollisionObjFilter, btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter ^ CCollisionObjFilter);
		m_isRegistPhysicsWorld = true;

		m_isInit = true;
	}

	void CCollisionObj::Register() {
		if (!m_isregistered && IsEnable()) {
			m_register = AddCollisionObj(this);
			m_isregistered = true;
		}
	}

}

	namespace {

		bool Masking(unsigned int group, unsigned int mask) {
			return (group & mask) != 0;
		}

		struct ObjManagerCallback : public btCollisionWorld::ContactResultCallback
		{
			Suicider::CCollisionObj* ObjA = nullptr;

			ObjManagerCallback(Suicider::CCollisionObj* A) : ObjA(A) {
				m_collisionFilterMask = CCollisionObjFilter;//CCollisionObj�Ƃ̂ݔ���
			};

			//���肷�邩�ǂ������肷��
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				//Bullet�̃}�X�N����
				bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
				collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
				if (!collides) { return false; }

				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)((btCollisionObject*)proxy0->m_clientObject)->getUserPointer();
				
				//�o�^����������ĂȂ���?
				if (!ObjB->IsEnable()) {
					return false;
				}
				
				//���̃��[�v�Ŋ��Ɏ��s�����g�ݍ��킹��?
				if (ObjA->GetIndex() > ObjB->GetIndex()) {
					return false;
				}

				//�}�X�N����
				if (!(Masking(ObjA->GetGroup(), ObjB->GetMask()) && Masking(ObjB->GetGroup(), ObjA->GetMask()))) {
					return false;
				}

				return true;
			};

			//�ڐG�̐��������s�����
			btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
			{

				Suicider::CCollisionObj* ObjA = (Suicider::CCollisionObj*)(colObj0Wrap->getCollisionObject()->getUserPointer());
				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)(colObj1Wrap->getCollisionObject()->getUserPointer());
				
				//�e�X�������s
				Suicider::CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetName(), ObjB->GetPointer(), ObjB->GetCollisionObject(), ObjB->GetClass(), false, cp };
				ObjA->RunCallback(paramB);
				Suicider::CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetName(), ObjA->GetPointer(), ObjA->GetCollisionObject(), ObjA->GetClass(), true, cp };
				ObjB->RunCallback(paramA);

				return 0.0f;
			};
		};
	}

	void CollisionObjManager::PostUpdate() {
		for (auto itr = m_colObjList.begin(); itr != m_colObjList.end(); ++itr) {

			Suicider::CCollisionObj* ObjA = (*itr).m_CObj;

			if (!ObjA->IsEnable() || !(*itr).m_isEnable) { continue; }

			ObjManagerCallback callback(ObjA);
			GetPhysicsWorld().ContactTest(&ObjA->GetCollisionObject(), callback);

		}
		m_colObjList.clear();
	}

}

}