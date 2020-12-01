#include "DWstdafx.h"
#include "RayTracingRender.h"
#include "Graphic/ReyTracing/ReyTracingEngine.h"

namespace DemolisherWeapon {

	void RayTracingRender::Init() {
		m_rayTracingEngine = &GetGraphicsEngine().GetDX12().GetRayTracingEngine();
		m_commandList = GetGraphicsEngine().GetDX12().GetCommandList4();
	}

	void RayTracingRender::Render() {
		Commit();
		m_rayTracingEngine->Dispatch(m_commandList);
	}

	void RayTracingRender::Commit() {
		m_rayTracingEngine->Update(m_commandList);
	}

}