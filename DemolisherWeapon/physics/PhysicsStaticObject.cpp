/*!
 * @brief	�ÓI�I�u�W�F�N�g�B
 */


#include "DWstdafx.h"
#include "physics/PhysicsStaticObject.h"

namespace DemolisherWeapon {

	void PhysicsStaticObject::Release()
	{
		GetEngine().GetPhysicsWorld().RemoveRigidBody(m_rigidBody);
	}
	void PhysicsStaticObject::CreateCommon(CVector3 pos, CQuaternion rot)
	{
		RigidBodyInfo rbInfo;
		rbInfo.collider = m_collider.get();
		rbInfo.mass = 0.0f;
		rbInfo.pos = pos;
		rbInfo.rot = rot;
		m_rigidBody.Create(rbInfo);
		GetEngine().GetPhysicsWorld().AddRigidBody(m_rigidBody);
	}

}