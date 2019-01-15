#include "DWstdafx.h"
#include "ShadowMapHandler.h"

namespace DemolisherWeapon {

	ShadowMapHandler::ShadowMapHandler()
	{
	}
	ShadowMapHandler::~ShadowMapHandler()
	{
		Release();
	}

	void ShadowMapHandler::Release() {
		if (m_shadowMap) {
			m_shadowMap->Release();
			m_shadowMap = nullptr;
		}
	}

	void ShadowMapHandler::Init(UINT width, UINT height, const CVector3& direction, const CVector3& target, const CVector3& area, float Near, const CVector3& dir_y) {
		m_shadowMap = CreateShadowMap(width, height);

		m_direction = direction; m_direction.Normalize();		
		m_target = target;
		m_area = area;

		if (dir_y.LengthSq() > FLT_EPSILON) {
			m_dirY = dir_y;
		}
		else {
			if (m_direction.y < 0.5f && m_direction.y > -0.5f) {
				m_dirY = CVector3::AxisY();
			}
			else {
				m_dirY = CVector3::AxisZ();
			}
		}
		m_dirY.Normalize();

		if (Near > FLT_EPSILON) {
			m_near = Near;
		}
		else {
			m_near = 10.0f;
		}
		
		ShadowMapUpdate();
	}

	void ShadowMapHandler::ShadowMapUpdate() {
		m_shadowMap->GetLightCamera().SetPos(m_target - m_direction * (m_area.z/2.0f));
		m_shadowMap->GetLightCamera().SetTarget(m_target);
		m_shadowMap->GetLightCamera().SetUp(m_dirY);
		m_shadowMap->GetLightCamera().SetNear(m_near);
		m_shadowMap->GetLightCamera().SetFar(m_area.z + 1.0f);
		m_shadowMap->GetLightCamera().SetWidth(m_area.x);
		m_shadowMap->GetLightCamera().SetHeight(m_area.y);
	}
}