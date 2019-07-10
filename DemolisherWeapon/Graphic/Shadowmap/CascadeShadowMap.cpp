#include "DWstdafx.h"
#include "CascadeShadowMap.h"

namespace DemolisherWeapon {
namespace GameObj {

	void CascadeShadowMap::Init(UINT width, UINT height, float areaNear, float areaFar){

		m_shadowMap = CreateShadowMap(width, height);
		SetCascadeNear(areaNear);
		SetCascadeFar(areaFar);

		m_lightCam = &m_shadowMap->GetLightCamera();

		m_isInit = true;
	}

	void CascadeShadowMap::Release() {
		if (m_shadowMap) {
			m_shadowMap->Release(); m_shadowMap = nullptr;
		}
		m_isInit = false;
	}

	CVector4 CascadeShadowMap::CreateAABB(float Near, float Far) {
		//視錐台の各方向
		CVector3 vZ = (GetMainCamera()->GetTarget() - GetMainCamera()->GetPos()).GetNorm();
		CVector3 vX; vX.Cross(GetMainCamera()->GetUp(), vZ); vX.Normalize();
		CVector3 vY; vY.Cross(vZ, vX); vY.Normalize();

		//アスペクト比と視野角の取得
		float aspect = GetMainCamera()->GetAspect();
		float fov = GetMainCamera()->GetFOV();

		if (fov < 0.0f) {
			//平行投影カメラの場合
		}

		//近平面の高さと幅
		float nearPlaneHalfHeight = tanf(fov * 0.5f) * Near;
		float nearPlaneHalfWidth = nearPlaneHalfHeight * aspect;

		//遠平面の高さと幅
		float farPlaneHalfHeight = tanf(fov * 0.5f) * Far;
		float farPlaneHalfWidth = farPlaneHalfHeight * aspect;

		//近・遠平面の中心座標
		CVector3 nearPlaneCenter = GetMainCamera()->GetPos() + vZ * Near;
		CVector3 farPlaneCenter = GetMainCamera()->GetPos() + vZ * Far;

		//視錐台の頂点を求める
		CVector3 corners[8];

		corners[0] = nearPlaneCenter - vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;
		corners[1] = nearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
		corners[2] = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
		corners[3] = nearPlaneCenter + vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;

		corners[4] = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
		corners[5] = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
		corners[6] = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
		corners[7] = farPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;

		//AABBを求める
		//CMatrix viewProj; viewProj.Mul(m_lightCam->GetViewMatrix(), m_lightCam->GetProjMatrix());
		CVector3 point = corners[0]; m_lightCam->GetViewMatrix().Mul(point); //viewProj.Mul(point);
		CVector2 mini = { point.x,point.y };
		CVector2 maxi = { point.x,point.y };
		for (int i = 1; i < 8; ++i)
		{
			point = corners[i]; m_lightCam->GetViewMatrix().Mul(point);
			mini = { min(mini.x,point.x),min(mini.y,point.y) };
			maxi = { max(maxi.x,point.x),max(maxi.y,point.y) };
		}

		return { mini.x, mini.y, maxi.x, maxi.y };
	}

	void CascadeShadowMap::PostLoopUpdate() {

		if (!m_isInit) { return; }

		CVector3 vZ = (GetMainCamera()->GetTarget() - GetMainCamera()->GetPos()).GetNorm();		

		float midv = (GetMainCamera()->GetFar()*m_cascadeAreaNear + GetMainCamera()->GetFar()*m_cascadeAreaFar)*0.5f;

		m_lightCam->SetPos( GetMainCamera()->GetPos() + vZ * midv + m_lightDir*-m_far*0.5f + m_lightDir*(-midv*max(vZ.Dot(m_lightDir),0.0f)) );
		m_lightCam->SetTarget(GetMainCamera()->GetPos() + vZ * midv);
		{
			//上方向求める
			CVector3 vZ = m_lightCam->GetFront();
			CVector3 vX; vX.Cross(m_lightCam->GetUp(), vZ);//右方向
			CVector3 vY; vY.Cross(vZ, vX);//上方向
			m_lightCam->SetUp(vY);
		}
		m_lightCam->SetNear(m_near);
		m_lightCam->SetFar(m_far);

		m_lightCam->UpdateMatrix();

		CVector4 minmax = CreateAABB(GetMainCamera()->GetFar()*m_cascadeAreaNear, GetMainCamera()->GetFar()*m_cascadeAreaFar);
		m_lightCam->SetProjMatMode(false);
		m_lightCam->Set4Point(minmax.x, minmax.z, minmax.y, minmax.w);
	}
}
}