#pragma once

namespace DemolisherWeapon {
namespace GameObj {

	class CascadeShadowMap : public IGameObject
	{
	public:
		CascadeShadowMap(float areaNear, float areaFar);
		~CascadeShadowMap();

		void Update()override;

		void SetPos(const CVector3& pos) { m_pos = pos; }
		void SetNear(float Near) { m_near = Near; }
		void SetFar(float Far) { m_far = Far; }
		void SetLightDir(const CVector3& dir) { m_lightDir = dir; }

	private:
		CVector4 CreateAABB(float Near, float Far);		

		CShadowMap* m_shadowMap = nullptr;
		GameObj::NoRegisterOrthoCamera* m_lightCam;//ライト視点カメラ

		CVector3 m_pos;
		CVector3 m_lightDir;
		float m_near, m_far;

		float m_cascadeAreaNear, m_cascadeAreaFar;
	};

}
}