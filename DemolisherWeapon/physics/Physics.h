#pragma once

namespace DemolisherWeapon{

class RigidBody;
class CCharacterController;

class CPhysicsWorld
{
	btDefaultCollisionConfiguration*		collisionConfig = nullptr;
	btCollisionDispatcher*					collisionDispatcher = nullptr;	//!<衝突解決処理。
	btBroadphaseInterface*					overlappingPairCache = nullptr;	//!<ブロードフェーズ。衝突判定の枝切り。
	btSequentialImpulseConstraintSolver*	constraintSolver = nullptr;		//!<コンストレイントソルバー。拘束条件の解決処理。
	btDiscreteDynamicsWorld*				dynamicWorld = nullptr;			//!<ワールド。
public:
	~CPhysicsWorld();
	void Init();
	void Update();
	void Release();
	/*!
	* @brief	重力を設定。。
	*/
	void SetGravity(CVector3 g)
	{
		dynamicWorld->setGravity(btVector3(g.x, g.y, g.z));
	}
	/*!
	* @brief	ダイナミックワールドを取得。
	*/
	btDiscreteDynamicsWorld* GetDynamicWorld()
	{
		return dynamicWorld;
	}
	/*!
	* @brief	剛体を登録。
	*/
	void AddRigidBody(RigidBody& rb);
	void AddRigidBody(RigidBody& rb, short group, short mask);
	/*!
	* @brief	剛体を破棄。
	*/
	void RemoveRigidBody(RigidBody& rb);
	/*!
	* @brief	コリジョンオブジェクトをワールドに登録。
	*@param[in]	colliObj	コリジョンオブジェクト。
	*/
	void AddCollisionObject(btCollisionObject& colliObj)
	{
		dynamicWorld->addCollisionObject(&colliObj);
	}
	/*!
	* @brief	コリジョンオブジェクトをワールドから削除。
	*@param[in]	colliObj	コリジョンオブジェクト。
	*/
	void RemoveCollisionObject(btCollisionObject& colliObj)
	{
		dynamicWorld->removeCollisionObject(&colliObj);
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
	//特定の2つオブジェクトの衝突検知
	void ContactPairTest(
		btCollisionObject* colObjA, btCollisionObject* colObjB,
		btCollisionWorld::ContactResultCallback& resultCallback
	)
	{
		dynamicWorld->contactPairTest(colObjA, colObjB, resultCallback);
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
