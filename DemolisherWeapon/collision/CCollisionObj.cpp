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

	void CCollisionObj::CreateCommon(const CVector3& pos, const CQuaternion& rot)
	{
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		m_ghostObject.setCollisionFlags(m_ghostObject.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);
		m_btOldTrans = m_ghostObject.getWorldTransform();

		m_ghostObject.setUserPointer(this);

		//�����G���W���ɓo�^
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter + CCollisionObjFilter, 0);// btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter ^ CCollisionObjFilter);
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

		struct ObjManagerCallback : public btCollisionWorld::ContactResultCallback
		{
			RegColObj* RegiObjA = nullptr;

			ObjManagerCallback(RegColObj* A) : RegiObjA(A) {
				m_collisionFilterMask = CCollisionObjFilter;//CCollisionObj�Ƃ̂ݔ���
			};

			//���肷�邩�ǂ������肷��
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				//�폜����ĂȂ���?
				if (proxy0 == nullptr || !RegiObjA->m_isEnable) {
					return false;
				}

				//Bullet�̃}�X�N����
				/*bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
				collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
				if (!collides) { return false; }*/
				
				//CCollisionObj�Ƃ̂ݔ���
				if (!(CCollisionObjFilter & proxy0->m_collisionFilterGroup)) {
					return false;
				}

				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)((btCollisionObject*)proxy0->m_clientObject)->getUserPointer();
				
				//�o�^����������ĂȂ���?
				if (!ObjB->IsEnable() || !RegiObjA->m_CObj->IsEnable()) {
					return false;
				}
				
				//���̃��[�v�Ŋ��Ɏ��s�����g�ݍ��킹��?
				if (ObjB->GetContactTestEnable() && RegiObjA->m_CObj->GetIndex() > ObjB->GetIndex()) {
					return false;
				}

				//�}�X�N����
				if (!(Masking(RegiObjA->m_CObj->GetGroup(), ObjB->GetMask()) && Masking(ObjB->GetGroup(), RegiObjA->m_CObj->GetMask()))) {
					return false;
				}

				return true;
			};

			//�ڐG�̐��������s�����
			btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
			{

				Suicider::CCollisionObj* ObjA = (Suicider::CCollisionObj*)(colObj0Wrap->getCollisionObject()->getUserPointer());
				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)(colObj1Wrap->getCollisionObject()->getUserPointer());
				
				bool old_hanteingA = ObjA->GetHanteing();
				bool old_hanteingB = ObjB->GetHanteing();

				ObjA->SetHanteing(true);
				ObjB->SetHanteing(true);

				//�e�X�������s
				Suicider::CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetName(), ObjB->GetPointer(), ObjB->GetCollisionObject(), ObjB->GetClass(), false, &cp };
				ObjA->RunCallback(paramB);
				Suicider::CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetName(), ObjA->GetPointer(), ObjA->GetCollisionObject(), ObjA->GetClass(), true, &cp };
				ObjB->RunCallback(paramA);
				
				ObjA->SetHanteing(old_hanteingA);
				ObjB->SetHanteing(old_hanteingB);

				return 0.0f;
			};
		};
		struct ObjManagerConvexCallback : public btCollisionWorld::ConvexResultCallback
		{
			RegColObj* RegiObjA = nullptr;

			ObjManagerConvexCallback(RegColObj* A) : RegiObjA(A) {
				m_collisionFilterMask = CCollisionObjFilter;//CCollisionObj�Ƃ̂ݔ���
			};

			//���肷�邩�ǂ������肷��
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				//�폜����ĂȂ���?
				if (proxy0 == nullptr || !RegiObjA->m_isEnable) {
					return false;
				}

				//CCollisionObj�Ƃ̂ݔ���
				if (!(CCollisionObjFilter & proxy0->m_collisionFilterGroup)) {
					return false;
				}

				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)((btCollisionObject*)proxy0->m_clientObject)->getUserPointer();

				//�����łȂ�
				if (RegiObjA->m_CObj == ObjB) {
					return false;
				}

				//�o�^����������ĂȂ���?
				if (!ObjB->IsEnable() || !RegiObjA->m_CObj->IsEnable()) {
					return false;
				}

				//���̃��[�v�Ŋ��Ɏ��s�����g�ݍ��킹��?
				if (ObjB->GetContactTestEnable() && RegiObjA->m_CObj->GetIndex() > ObjB->GetIndex()) {
					return false;
				}

				//�}�X�N����
				if (!(Masking(RegiObjA->m_CObj->GetGroup(), ObjB->GetMask()) && Masking(ObjB->GetGroup(), RegiObjA->m_CObj->GetMask()))) {
					return false;
				}

				return true;
			};

			//�ڐG�̐��������s�����
			btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)override
			{				
				Suicider::CCollisionObj* ObjA = RegiObjA->m_CObj;
				Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)(convexResult.m_hitCollisionObject->getUserPointer());

				bool old_hanteingA = ObjA->GetHanteing();
				bool old_hanteingB = ObjB->GetHanteing();

				ObjA->SetHanteing(true);
				ObjB->SetHanteing(true);

				//�e�X�������s
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

			ObjA->SetHanteing(true);//���蒆!

			ObjManagerCallback callback(&(*itr));
			GetPhysicsWorld().ContactTest(&ObjA->GetCollisionObject(), callback);
			//ObjManagerConvexCallback callback(&(*itr));
			//GetPhysicsWorld().ConvexSweepTest((const btConvexShape*)ObjA->GetCollisionObject().getCollisionShape(), ObjA->GetBtOldTrans(), ObjA->GetCollisionObject().getWorldTransform(), callback);

			ObjA->SetHanteing(false);//����I���
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