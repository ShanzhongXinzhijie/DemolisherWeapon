#include "DWstdafx.h"
#include "PhysicsDebugDrawRender.h"

namespace DemolisherWeapon {

	PhysicsDebugDrawRender::PhysicsDebugDrawRender()
	{
	}
	PhysicsDebugDrawRender::~PhysicsDebugDrawRender()
	{
		Release();
	}

	void PhysicsDebugDrawRender::Init() {
		if (!m_isInit) {
			m_isInit = true;
			m_physicsDrawer.Init();
			GetEngine().GetPhysicsWorld().GetDynamicWorld()->setDebugDrawer(&m_physicsDrawer);
		}
	}
	void PhysicsDebugDrawRender::Release() {

	}

	void  PhysicsDebugDrawRender::Render() {
		if (m_enable) {
			//�����_�[�^�[�Q�b�g�Ƃ��ݒ�		
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
				1,
				&GetEngine().GetGraphicsEngine().GetFRT().GetRTV(),
				GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
			);

			//�`�悵�Ȃ���
			m_physicsDrawer.BeginDraw();
			GetEngine().GetPhysicsWorld().GetDynamicWorld()->debugDrawWorld();
			m_physicsDrawer.EndDraw();

			//�����_�[�^�[�Q�b�g����
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
		}
	}
}