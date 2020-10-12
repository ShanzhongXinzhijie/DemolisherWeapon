#include "DWstdafx.h"
#include "CCollisionObj.h"
#include "physics/CollisionAttr.h"
#include "bulletPhysics/src/LinearMath/btVector3.h"

namespace DemolisherWeapon {

namespace GameObj {

namespace Suicider {

	void CCollisionObj::Release()
	{
		//�����G���W������폜
		if (m_isRegistPhysicsWorld == true) {
			GetEngine().GetPhysicsWorld().RemoveCollisionObject(m_ghostObject);
			m_isRegistPhysicsWorld = false;
		}
	}

	void CCollisionObj::CreateCommon(const CVector3& pos, const CQuaternion& rot)
	{
		//�S�[�X�g�I�u�W�F�N�g�̐ݒ�
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		m_ghostObject.setCollisionFlags(m_ghostObject.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		m_ghostObject.setUserIndex(enCollisionAttr_CCollisionObj);
		m_ghostObject.setUserPointer(this);
		
		//�g�����X�t�H�[���̐ݒ�
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);
		m_btOldTrans = m_ghostObject.getWorldTransform();
		
		//�Փ˃}�X�N
		short int mask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter;//StaticFilter�ȊO�Ɣ���
		if (!m_isStaticObj) {
			mask = CCollisionObjFilter;//CCollisionObjFilter�Ƃ̂ݔ���
		}

		//�����G���W���ɓo�^
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter | CCollisionObjFilter, mask);
		m_isRegistPhysicsWorld = true;

		//����������
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

		const btGhostObject dummy;//�_�~�[�p

		bool NeedsCollision(btBroadphaseProxy* proxy0, RegColObj* RegiObjA, short int	m_collisionFilterGroup, short int	m_collisionFilterMask) {
			//�폜����ĂȂ���?
			if (proxy0 == nullptr || !RegiObjA->m_isEnable) {
				return false;
			}

			//Bullet�̃}�X�N����
			bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
			collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			if (!collides) { return false; }

			//CCollisionObj�łȂ���΂�����true
			if (!(proxy0->m_collisionFilterGroup & CCollisionObjFilter)) {
				return true;
			}

			Suicider::CCollisionObj* ObjB = (Suicider::CCollisionObj*)((btCollisionObject*)proxy0->m_clientObject)->getUserPointer();

			//�o�^����������ĂȂ���?
			if (!ObjB->IsEnable() || !RegiObjA->m_CObj->IsEnable()) {
				return false;
			}

			//A���n�C�X�s�[�h����Ȃ��āc
			if (!RegiObjA->m_CObj->GetIsHighSpeed()) {
				//���̃��[�v�Ŋ��Ɏ��s�����g�ݍ��킹��?
				//B�����炢���肶��Ȃ��ĉߋ��ɏ������s���Ă���return
				if (ObjB->GetContactTestEnable() && RegiObjA->m_CObj->GetIndex() > ObjB->GetIndex()) {
					return false;
				}
			}

			//�}�X�N����
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
				//�e�X�������s
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

			//�R���X�g���N�^
			ObjManagerCallback(RegColObj* A, bool isCollisionStaticObject) : RegiObjA(A) {
				if (isCollisionStaticObject) {
					m_collisionFilterMask = CCollisionObjFilter | btBroadphaseProxy::StaticFilter;//CCollisionObj�ƐÓI�I�u�W�F�N�g�̂ݔ���
				}
				else {
					m_collisionFilterMask = CCollisionObjFilter;//CCollisionObj�Ƃ̂ݔ���
				}
				m_collisionFilterGroup = btBroadphaseProxy::StaticFilter | CCollisionObjFilter;
			}

			//���肷�邩�ǂ������肷��
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				return NeedsCollision(proxy0, RegiObjA, m_collisionFilterGroup, m_collisionFilterMask);
			}

			//�ڐG�̐��������s�����
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

			//�R���X�g���N�^
			ObjManagerConvexCallback(RegColObj* A, bool isCollisionStaticObject) : RegiObjA(A) {
				if (isCollisionStaticObject) {
					m_collisionFilterMask = CCollisionObjFilter | btBroadphaseProxy::StaticFilter;//CCollisionObj�ƐÓI�I�u�W�F�N�g�̂ݔ���
				}
				else {
					m_collisionFilterMask = CCollisionObjFilter;//CCollisionObj�Ƃ̂ݔ���
				}
				m_collisionFilterGroup = btBroadphaseProxy::StaticFilter | CCollisionObjFilter;
			};

			//���肷�邩�ǂ������肷��
			bool needsCollision(btBroadphaseProxy* proxy0) const override
			{
				//���Ԃ�łȂ�
				if ((btCollisionObject*)proxy0->m_clientObject == (btCollisionObject*)&(RegiObjA->m_CObj->GetCollisionObject())) {
					return false;
				}
				return NeedsCollision(proxy0, RegiObjA, m_collisionFilterGroup, m_collisionFilterMask);
			};

			//�ڐG�̐��������s�����
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

			ObjA->SetHanteing(true);//���蒆!

			if (ObjA->GetIsHighSpeed() && !(ObjA->GetBtOldTrans().getOrigin() - ObjA->GetCollisionObject().getWorldTransform().getOrigin()).fuzzyZero()) {
				ObjManagerConvexCallback callback(&(*itr), ObjA->GetIsCollisionStaticObject());
				GetPhysicsWorld().ConvexSweepTest((const btConvexShape*)ObjA->GetCollisionObject().getCollisionShape(), ObjA->GetBtOldTrans(), ObjA->GetCollisionObject().getWorldTransform(), callback);
			}
			else {
				ObjManagerCallback callback(&(*itr), ObjA->GetIsCollisionStaticObject());
				GetPhysicsWorld().ContactTest(&ObjA->GetCollisionObject(), callback);
			}
			
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