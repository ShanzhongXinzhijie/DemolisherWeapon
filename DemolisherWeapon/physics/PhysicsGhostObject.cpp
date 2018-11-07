#include "DWstdafx.h"
#include "PhysicsGhostObject.h"

namespace DemolisherWeapon {
	void PhysicsGhostObject::Release()
	{
		if (m_isRegistPhysicsWorld == true) {
			GetEngine().GetPhysicsWorld().RemoveCollisionObject(m_ghostObject);
			m_isRegistPhysicsWorld = false;
		}
	}
	void PhysicsGhostObject::CreateCommon(CVector3 pos, CQuaternion rot)
	{
		m_ghostObject.setCollisionShape(m_collider->GetBody());
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);

		//�����G���W���ɓo�^�B
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject);
		m_isRegistPhysicsWorld = true;
	}
}