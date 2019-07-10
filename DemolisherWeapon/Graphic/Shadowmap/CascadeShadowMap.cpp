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
		//������̊e����
		CVector3 vZ = (GetMainCamera()->GetTarget() - GetMainCamera()->GetPos()).GetNorm();
		CVector3 vX; vX.Cross(GetMainCamera()->GetUp(), vZ); vX.Normalize();
		CVector3 vY; vY.Cross(vZ, vX); vY.Normalize();

		//�A�X�y�N�g��Ǝ���p�̎擾
		float aspect = GetMainCamera()->GetAspect();
		float fov = GetMainCamera()->GetFOV();

		if (fov < 0.0f) {
			//���s���e�J�����̏ꍇ
		}

		//�ߕ��ʂ̍����ƕ�
		float nearPlaneHalfHeight = tanf(fov * 0.5f) * Near;
		float nearPlaneHalfWidth = nearPlaneHalfHeight * aspect;

		//�����ʂ̍����ƕ�
		float farPlaneHalfHeight = tanf(fov * 0.5f) * Far;
		float farPlaneHalfWidth = farPlaneHalfHeight * aspect;

		//�߁E�����ʂ̒��S���W
		CVector3 nearPlaneCenter = GetMainCamera()->GetPos() + vZ * Near;
		CVector3 farPlaneCenter = GetMainCamera()->GetPos() + vZ * Far;

		//������̒��_�����߂�
		CVector3 corners[8];

		corners[0] = nearPlaneCenter - vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;
		corners[1] = nearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
		corners[2] = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
		corners[3] = nearPlaneCenter + vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;

		corners[4] = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
		corners[5] = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
		corners[6] = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
		corners[7] = farPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;

		//AABB�����߂�
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
			//��������߂�
			CVector3 vZ = m_lightCam->GetFront();
			CVector3 vX; vX.Cross(m_lightCam->GetUp(), vZ);//�E����
			CVector3 vY; vY.Cross(vZ, vX);//�����
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