#include "DWstdafx.h"
#include "PhysicsDynamicObject.h"

namespace DemolisherWeapon {

	void PhysicsDynamicObject::Release()
	{
		GetEngine().GetPhysicsWorld().RemoveRigidBody(m_rigidBody);
	}
	void PhysicsDynamicObject::CreateCommon(CVector3 pos, CQuaternion rot)
	{
		RigidBodyInfo rbInfo;
		rbInfo.collider = m_collider.get();
		rbInfo.mass = m_mass;
		rbInfo.pos = pos;
		rbInfo.rot = rot;
		rbInfo.localInteria = m_localInterial;
		m_rigidBody.Create(rbInfo);
		GetEngine().GetPhysicsWorld().AddRigidBody(m_rigidBody);
	}

}