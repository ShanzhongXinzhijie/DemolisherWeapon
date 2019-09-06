#pragma once

namespace DemolisherWeapon{

class RigidBody;
class CCharacterController;

class CPhysicsWorld
{
	btDefaultCollisionConfiguration*		collisionConfig = nullptr;
	btCollisionDispatcher*					collisionDispatcher = nullptr;	//!<�Փˉ��������B
	btBroadphaseInterface*					overlappingPairCache = nullptr;	//!<�u���[�h�t�F�[�Y�B�Փ˔���̎}�؂�B
	btSequentialImpulseConstraintSolver*	constraintSolver = nullptr;		//!<�R���X�g���C���g�\���o�[�B�S�������̉��������B
	btSoftRigidDynamicsWorld*				dynamicWorld = nullptr;			//!<���[���h�B
	btSoftBodyWorldInfo*					softBodyWorldInfo = nullptr;	//�\�t�g�{�f�B�[�Ǘ����
public:
	~CPhysicsWorld();
	void Init();
	void Update();
	void Release();
	/*!
	* @brief	�d�͂�ݒ�B�B
	*/
	void SetGravity(CVector3 g)
	{
		dynamicWorld->setGravity(btVector3(g.x, g.y, g.z));
	}
	/*!
	* @brief	�_�C�i�~�b�N���[���h���擾�B
	*/
	btSoftRigidDynamicsWorld* GetDynamicWorld()
	{
		return dynamicWorld;
	}
	/// <summary>
	/// �\�t�g�{�f�B�̊Ǘ������擾
	/// </summary>
	btSoftBodyWorldInfo* GetSoftBodyWorldInfo()
	{
		return softBodyWorldInfo;
	}
	/*!
	* @brief	���̂�o�^�B
	*/
	void AddRigidBody(RigidBody& rb);
	void AddRigidBody(RigidBody& rb, short group, short mask);
	/*!
	* @brief	���̂�j���B
	*/
	void RemoveRigidBody(RigidBody& rb);
	/*!
	* @brief	�R���W�����I�u�W�F�N�g�����[���h�ɓo�^�B
	*@param[in]	colliObj	�R���W�����I�u�W�F�N�g�B
	*/
	/*void AddCollisionObject(btCollisionObject& colliObj)
	{
		dynamicWorld->addCollisionObject(&colliObj);
	}*/
	void AddCollisionObject(btCollisionObject& colliObj, short int group = btBroadphaseProxy::StaticFilter, short int mask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter)
	{
		dynamicWorld->addCollisionObject(&colliObj, group, mask);
	}
	/*!
	* @brief	�R���W�����I�u�W�F�N�g�����[���h����폜�B
	*@param[in]	colliObj	�R���W�����I�u�W�F�N�g�B
	*/
	void RemoveCollisionObject(btCollisionObject& colliObj)
	{
		if (dynamicWorld) { dynamicWorld->removeCollisionObject(&colliObj); }
	}

	void ConvexSweepTest(
		const btConvexShape* castShape,
		const btTransform& convexFromWorld,
		const btTransform& convexToWorld,
		btCollisionWorld::ConvexResultCallback& resultCallback,
		btScalar allowedCcdPenetration = 0.0f
	)
	{
		dynamicWorld->convexSweepTest(castShape, convexFromWorld, convexToWorld, resultCallback, allowedCcdPenetration);
	}
	void ContactTest(
		btCollisionObject* colObj, 
		btCollisionWorld::ContactResultCallback& resultCallback
	)
	{
		dynamicWorld->contactTest(colObj, resultCallback);
	}
	//�����2�I�u�W�F�N�g�̏Փˌ��m
	//�d��!!!!!!
	void ContactPairTest(
		btCollisionObject* colObjA, btCollisionObject* colObjB,
		btCollisionWorld::ContactResultCallback& resultCallback
	)
	{
		dynamicWorld->contactPairTest(colObjA, colObjB, resultCallback);
	}
	//���C�Ŕ���
	void RayTest(
		const btVector3& rayFromWorld, const btVector3& rayToWorld,
		btCollisionWorld::RayResultCallback& resultCallback) const {

		dynamicWorld->rayTest(rayFromWorld, rayToWorld, resultCallback);
	}

	void ContactTest(
		btCollisionObject* colObj,
		std::function<void(const btCollisionObject& contactCollisionObject)> cb
	);
	void ContactTest(
		RigidBody& rb,
		std::function<void(const btCollisionObject& contactCollisionObject)> cb
	);
	void ContactTest(
		CCharacterController& charaCon,
		std::function<void(const btCollisionObject& contactCollisionObject)> cb
	);
};

}
