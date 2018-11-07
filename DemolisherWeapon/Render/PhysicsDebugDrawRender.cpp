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
		m_physicsDrawer.Init();
		GetEngine().GetPhysicsWorld().GetDynamicWorld()->setDebugDrawer(&m_physicsDrawer);		
	}
	void PhysicsDebugDrawRender::Release() {

	}

	void  PhysicsDebugDrawRender::Render() {
		//レンダーターゲットとか設定		
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
			1,
			&GetEngine().GetGraphicsEngine().GetFinalRender().GetFRT().GetRTV(), 
			GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
		);

		//描画しなさい
		m_physicsDrawer.BeginDraw();
		GetEngine().GetPhysicsWorld().GetDynamicWorld()->debugDrawWorld();
		m_physicsDrawer.EndDraw();
	}
}