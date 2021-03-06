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
			//GPUイベントの開始
			GetGraphicsEngine().BeginGPUEvent(L"PhysicsDebugDrawRender");
			
			//レンダーターゲットとか設定		
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
				1,
				&GetEngine().GetGraphicsEngine().GetFRT().GetRTV(),
				GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
			);

			//描画しなさい
			m_physicsDrawer.BeginDraw();
			GetEngine().GetPhysicsWorld().GetDynamicWorld()->debugDrawWorld();
			m_physicsDrawer.EndDraw();

			//レンダーターゲット解除
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

			//GPUイベントの終了
			GetGraphicsEngine().EndGPUEvent();
		}
	}
}