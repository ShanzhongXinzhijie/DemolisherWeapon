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
		m_ghostObject.setCollisionFlags(m_ghostObject.getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		btTransform btTrans;
		btTrans.setOrigin({ pos.x, pos.y, pos.z });
		btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
		m_ghostObject.setWorldTransform(btTrans);

		//ï®óùÉGÉìÉWÉìÇ…ìoò^ÅB
		GetEngine().GetPhysicsWorld().AddCollisionObject(m_ghostObject);
		m_isRegistPhysicsWorld = true;
	}
}