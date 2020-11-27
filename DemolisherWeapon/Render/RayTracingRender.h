#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class RayTracingEngine;

	class RayTracingRender :
		public IRander
	{
	public:
		RayTracingRender() = default;
		//~RayTracingRender() { Release(); }

		void Init();
		//void Release();

		//void Resize()override;

		void Render()override;
		//void PostRender()override;

		void Commit();

	private:
		//bool m_isTestInited = false;
		RayTracingEngine* m_rayTracingEngine = nullptr;
		ID3D12GraphicsCommandList4* m_commandList = nullptr;
	};

}