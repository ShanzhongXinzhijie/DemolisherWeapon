#include "DWstdafx.h"
#include "physics/Physics.h"
#include "Physics/RigidBody.h"
#include "Physics/character/CCharacterController.h"

#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"

#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"

namespace DemolisherWeapon {

namespace {
	struct MyContactResultCallback : public btCollisionWorld::ContactResultCallback {
		using ContantTestCallback = std::function<void(const btCollisionObject& contactCollisionObject)>;
		ContantTestCallback  m_cb;
		btCollisionObject* m_me = nullptr;
		virtual	btScalar	addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
		{
			if (m_me == colObj0Wrap->getCollisionObject()) {
				m_cb(*colObj1Wrap->getCollisionObject());
			}
			return 0.0f;
		}
	};

	bool CustomMaterialCombinerCallback(btManifoldPoint& cp,
		const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
		const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
	{
		btAdjustInternalEdgeContacts(cp, colObj1Wrap, colObj0Wrap, partId1, index1);
		return true;
	}
}

//extern ContactAddedCallback gContactAddedCallback;

CPhysicsWorld::~CPhysicsWorld()
{
	Release();
}
void CPhysicsWorld::Release()
{
	if (softBodyWorldInfo) { softBodyWorldInfo->m_sparsesdf.Reset(); }

	delete dynamicWorld;
	delete constraintSolver;
	delete overlappingPairCache;
	delete collisionDispatcher;
	delete collisionConfig;
	delete softBodyWorldInfo;

	dynamicWorld = nullptr;
	constraintSolver = nullptr;
	overlappingPairCache = nullptr;
	collisionDispatcher = nullptr;
	collisionConfig = nullptr;
	softBodyWorldInfo = nullptr;
}
void CPhysicsWorld::Init()
{
	Release();

	// btAdjustInternalEdgeContacts
	gContactAddedCallback = CustomMaterialCombinerCallback;

	//物理エンジンを初期化。
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfig = new btSoftBodyRigidBodyCollisionConfiguration();// btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	collisionDispatcher = new	btCollisionDispatcher(collisionConfig);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	constraintSolver = new btSequentialImpulseConstraintSolver;

	//ダイナミックワールドを初期化
	dynamicWorld = new btSoftRigidDynamicsWorld(
		collisionDispatcher,
		overlappingPairCache,
		constraintSolver,
		collisionConfig
		);

	//重力を設定
	dynamicWorld->setGravity(btVector3(0, -10, 0));

	// ソフトボディー管理情報
	// align16 SIMD計算 
	softBodyWorldInfo = new(_aligned_malloc(sizeof(btSoftBodyWorldInfo), __alignof(btSoftBodyWorldInfo))) btSoftBodyWorldInfo;
	softBodyWorldInfo->m_dispatcher = collisionDispatcher;
	softBodyWorldInfo->m_broadphase = overlappingPairCache;
	softBodyWorldInfo->m_gravity = dynamicWorld->getGravity();
	softBodyWorldInfo->m_sparsesdf.Initialize();
}
void CPhysicsWorld::Update()
{
	dynamicWorld->stepSimulation(1.0f/ GetStandardFrameRate());
}
void CPhysicsWorld::AddRigidBody(RigidBody& rb)
{
	if (rb.IsAddPhysicsWorld() == false) {
		dynamicWorld->addRigidBody(rb.GetBody());
		rb.SetMarkAddPhysicsWorld();
	}
}
void CPhysicsWorld::AddRigidBody(RigidBody& rb, short group, short mask)
{
	if (rb.IsAddPhysicsWorld() == false) {
		dynamicWorld->addRigidBody(rb.GetBody(), group, mask);
		rb.SetMarkAddPhysicsWorld();
	}
}
void CPhysicsWorld::RemoveRigidBody(RigidBody& rb)
{
	if (rb.IsAddPhysicsWorld() == true) {
		if (dynamicWorld) { dynamicWorld->removeRigidBody(rb.GetBody()); }
		rb.SetUnmarkAddPhysicsWorld();
	}
}

void CPhysicsWorld::ContactTest(
	btCollisionObject* colObj,
	std::function<void(const btCollisionObject& contactCollisionObject)> cb
) {
	MyContactResultCallback myContactResultCallback;
	myContactResultCallback.m_cb = cb;
	myContactResultCallback.m_me = colObj;
	dynamicWorld->contactTest(colObj, myContactResultCallback);
}

void CPhysicsWorld::ContactTest(
	RigidBody& rb,
	std::function<void(const btCollisionObject& contactCollisionObject)> cb
)
{
	ContactTest(rb.GetBody(), cb);
}
void CPhysicsWorld::ContactTest(
	CCharacterController& charaCon,
	std::function<void(const btCollisionObject& contactCollisionObject)> cb
)
{
	ContactTest(*charaCon.GetRigidBody(), cb);
}

}