#include "DWstdafx.h"
#include "CCollisionObj.h"

namespace DemolisherWeapon {

namespace GameObj {

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

		//�����G���W���ɓo�^�B(�f�o�b�O�\���̂���)
#ifndef DW_MASTER	
		//if (GetEnablePhysicsDebugDraw()) {
			//mask=0�ɂ��Ƃ�
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject);// , 2, 0);
										// �f�t�H���g���ƃX�^�e�B�b�N�I�u�W�F�ɑ����A�X�^�e�B�b�N�Ƃ̂ݔ���
			m_isRegistPhysicsWorld = true;
		//}
#endif

		m_isInit = true;
	}

	void CCollisionObj::Register() {
		if (!m_isregistered) {
			m_register = AddCollisionObj(this);
			m_isregistered = true;
		}
	}

	namespace {

		struct ObjManagerCallback : public btCollisionWorld::ContactResultCallback
		{
			CCollisionObj* ObjA = nullptr, *ObjB = nullptr;

			ObjManagerCallback(CCollisionObj* A, CCollisionObj* B) : ObjA(A), ObjB(B) {
				m_collisionFilterMask = 2;//2(�R���W�����ƃL�����R���̔��肷�邩����Őݒ�)
				//contactTest��ContactTest(&ObjA->GetCollisionObject()�̃}�X�N�E�O���[�v�͂���ł����!
			};

			//needsCollision 

			virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)override
			{

				//�e�X�������s
				CCollisionObj::SCallbackParam paramB = { ObjB->GetNameKey(), ObjB->GetData(), ObjB->GetCollisionObject(), ObjB->GetClass() };
				ObjA->RunCallback(paramB);
				CCollisionObj::SCallbackParam paramA = { ObjA->GetNameKey(), ObjA->GetData(), ObjA->GetCollisionObject(), ObjA->GetClass() };
				ObjB->RunCallback(paramA);

				//colObj0Wrap->getCollisionObject()->getUserPointer

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

			CCollisionObj* ObjA = (*itr).m_CObj;

			if (!ObjA->IsEnable() || !(*itr).m_isEnable) { continue; }

			ObjManagerCallback callback(ObjA, ObjA);
			GetPhysicsWorld().ContactTest(&ObjA->GetCollisionObject(), callback);
			//����ōs����!
			//ObjA->GetCollisionObject().setUserPointer();

			/*auto itr2 = itr; ++itr2;
			for (itr2; itr2 != m_colObjList.end(); ++itr2) {

				CCollisionObj* ObjB = (*itr2).m_CObj;

				if (!ObjB->IsEnable() || !(*itr2).m_isEnable) { continue; }

				if (Masking(ObjA->GetGroup(),ObjB->GetMask()) && Masking(ObjB->GetGroup(), ObjA->GetMask())) {//�}�X�L���O
					ObjManagerCallback callback(ObjA, ObjB);
					GetPhysicsWorld().ContactPairTest(&ObjA->GetCollisionObject(), &ObjB->GetCollisionObject(), callback);
				}
			}*/
		}
		//}
		m_colObjList.clear();
	}

}

}