/*!
 * @brief	静的オブジェクト。
 */


#include "DWstdafx.h"
#include "physics/PhysicsStaticObject.h"

namespace DemolisherWeapon {

PhysicsStaticObject::PhysicsStaticObject()
{
}
PhysicsStaticObject::~PhysicsStaticObject()
{
	GetEngine().GetPhysicsWorld().RemoveRigidBody(m_rigidBody);
}

void PhysicsStaticObject::CreateMeshObject(SkinModel& skinModel, CVector3 pos, CQuaternion rot, const CMatrix* offsetMatrix)
{
	//メッシュコライダーを作成。
	m_meshCollider.CreateFromSkinModel(skinModel, offsetMatrix);
	//剛体を作成、
	RigidBodyInfo rbInfo;
	rbInfo.collider = &m_meshCollider; //剛体に形状(コライダー)を設定する。
	rbInfo.mass = 0.0f;
	rbInfo.pos = pos;
	rbInfo.rot = rot;
	m_rigidBody.Create(rbInfo);
	//剛体を物理ワールドに追加する。
	GetEngine().GetPhysicsWorld().AddRigidBody(m_rigidBody);
}

}