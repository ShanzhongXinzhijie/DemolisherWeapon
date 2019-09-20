/*!
 * @brief	静的オブジェクト。
 */


#include "DWstdafx.h"
#include "physics/PhysicsStaticObject.h"

namespace DemolisherWeapon {

	void PhysicsStaticObject::Release()
	{
		if (m_isAdd) {
			GetEngine().GetPhysicsWorld().RemoveRigidBody(m_rigidBody);
			m_isAdd = false;
		}
	}
	void PhysicsStaticObject::CreateCommon(const CVector3& pos, const CQuaternion& rot)
	{
		RigidBodyInfo rbInfo;
		rbInfo.collider = m_collider.get();
		rbInfo.mass = 0.0f;
		rbInfo.pos = pos;
		rbInfo.rot = rot;
		m_rigidBody.Create(rbInfo);
		GetEngine().GetPhysicsWorld().AddRigidBody(m_rigidBody);
		
		m_isAdd = true;
	}

}