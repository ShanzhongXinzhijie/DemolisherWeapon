#include "DWstdafx.h"
#include "Physics/RigidBody.h"
#include "Physics/ICollider.h"

namespace DemolisherWeapon {

RigidBody::~RigidBody()
{
	Release();
}
void RigidBody::Release()
{
	delete rigidBody;
	delete myMotionState;
	rigidBody = nullptr;
	myMotionState = nullptr;
}
void RigidBody::Create(RigidBodyInfo& rbInfo)
{

	Release();
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(rbInfo.pos.x, rbInfo.pos.y, rbInfo.pos.z));
	transform.setRotation(btQuaternion(rbInfo.rot.x, rbInfo.rot.y, rbInfo.rot.z, rbInfo.rot.w));
	myMotionState = new btDefaultMotionState;
	myMotionState->setWorldTransform(transform);
	btRigidBody::btRigidBodyConstructionInfo btRbInfo(rbInfo.mass, myMotionState, rbInfo.collider->GetBody(), btVector3(0, 0, 0));
	//„‘Ì‚ğì¬B
	rigidBody = new btRigidBody(btRbInfo);

}

}