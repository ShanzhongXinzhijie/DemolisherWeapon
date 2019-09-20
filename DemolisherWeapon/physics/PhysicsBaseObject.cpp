#include "DWstdafx.h"
#include "PhysicsBaseObject.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "MeshCollider.h"

namespace DemolisherWeapon {

	void PhysicsBaseObject::CreateBox(const CVector3& pos, const CQuaternion& rot, const CVector3& size)
	{
		Release();
		auto boxCollider = std::make_unique<BoxCollider>();
		boxCollider->Create(size);
		m_collider = std::move(boxCollider);
		CreateCommon(pos, rot);
	}
	void PhysicsBaseObject::CreateCapsule(const CVector3& pos, const CQuaternion& rot, float radius, float height)
	{
		Release();
		auto capusuleCollider = std::make_unique<CapsuleCollider>();
		capusuleCollider->Create(radius, height);
		m_collider = std::move(capusuleCollider);
		CreateCommon(pos, rot);
	}

	void PhysicsBaseObject::CreateSphere(const CVector3& pos, const CQuaternion& rot, float radius)
	{
		Release();
		auto sphereCollider = std::make_unique<SphereCollider>();
		sphereCollider->Create(radius);
		m_collider = std::move(sphereCollider);
		CreateCommon(pos, rot);
	}
	void PhysicsBaseObject::CreateMesh(const GameObj::CSkinModelRender& skinModelRender)
	{
		CreateMesh(skinModelRender.GetPos(), skinModelRender.GetRot(), skinModelRender.GetScale(), skinModelRender.GetSkinModel());
	}
	void PhysicsBaseObject::CreateMesh(const CVector3& pos, const CQuaternion& rot, const CVector3& scale, const SkinModel& skinModel)
	{
		Release();
		CMatrix mScale;
		mScale.MakeScaling(scale);
		auto meshCollider = std::make_unique<MeshCollider>();
		meshCollider->CreateFromSkinModel(skinModel, &mScale);
		m_collider = std::move(meshCollider);
		CreateCommon(
			pos,
			rot
		);

	}

}