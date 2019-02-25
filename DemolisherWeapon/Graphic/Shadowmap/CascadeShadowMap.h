#pragma once

namespace DemolisherWeapon {
namespace GameObj {

	class CascadeShadowMap : public IGameObject
	{
	public:
		CascadeShadowMap() = default;
		CascadeShadowMap(UINT width, UINT height, float areaNear, float areaFar) { Init(width, height, areaNear, areaFar); }
		~CascadeShadowMap() { Release(); }

		void Init(UINT width, UINT height, float areaNear, float areaFar);
		void Release();

		void PostLoopUpdate()override;

		void SetPos(const CVector3& pos) { m_pos = pos; }
		void SetNear(float Near) { m_near = Near; }
		void SetFar(float Far) { m_far = Far; }
		void SetLightDir(const CVector3& dir) { m_lightDir = dir; }

		void SetCascadeNear(float Near) { m_cascadeAreaNear = Near; m_shadowMap->SetCascadeNear(m_cascadeAreaNear); }
		void SetCascadeFar(float Far) { m_cascadeAreaFar = Far; m_shadowMap->SetCascadeFar(m_cascadeAreaFar); }

	private:
		CVector4 CreateAABB(float Near, float Far);		

		bool m_isInit = false;

		CShadowMap* m_shadowMap = nullptr;
		GameObj::NoRegisterOrthoCamera* m_lightCam;//ライト視点カメラ

		CVector3 m_pos;
		CVector3 m_lightDir = { 0.0f,-1.0f,0.0f };
		float m_near = 50.0f, m_far = 20000.0f;

		float m_cascadeAreaNear, m_cascadeAreaFar;
	};

}
}