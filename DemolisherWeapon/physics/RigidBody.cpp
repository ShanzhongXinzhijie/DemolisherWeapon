#include "DWstdafx.h"
#include "Physics/RigidBody.h"
#include "Physics/ICollider.h"

namespace DemolisherWeapon {
	
	void RigidBody::Create(RigidBodyInfo& rbInfo)
	{
		Release();
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(rbInfo.pos.x, rbInfo.pos.y, rbInfo.pos.z));
		transform.setRotation(btQuaternion(rbInfo.rot.x, rbInfo.rot.y, rbInfo.rot.z, rbInfo.rot.w));
		m_myMotionState = std::make_unique<btDefaultMotionState>();
		m_myMotionState->setWorldTransform(transform);
		btVector3 btLocalInteria;
		rbInfo.localInteria.CopyTo(btLocalInteria);
		btRigidBody::btRigidBodyConstructionInfo btRbInfo(rbInfo.mass, m_myMotionState.get(), rbInfo.collider->GetBody(), btLocalInteria);
		//çÑëÃÇçÏê¨ÅB
		m_rigidBody = std::make_unique<btRigidBody>(btRbInfo);
		
		//btAdjustInternalEdgeContactsÇÃÇ‚Ç¬
		m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	}

}