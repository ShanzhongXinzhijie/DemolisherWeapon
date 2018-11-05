/*!
 * @brief	�ÓI�I�u�W�F�N�g�B
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
	//���b�V���R���C�_�[���쐬�B
	m_meshCollider.CreateFromSkinModel(skinModel, offsetMatrix);
	//���̂��쐬�A
	RigidBodyInfo rbInfo;
	rbInfo.collider = &m_meshCollider; //���̂Ɍ`��(�R���C�_�[)��ݒ肷��B
	rbInfo.mass = 0.0f;
	rbInfo.pos = pos;
	rbInfo.rot = rot;
	m_rigidBody.Create(rbInfo);
	//���̂𕨗����[���h�ɒǉ�����B
	GetEngine().GetPhysicsWorld().AddRigidBody(m_rigidBody);
}

}