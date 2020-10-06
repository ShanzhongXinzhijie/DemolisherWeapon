#pragma once

#include "IRander.h"
#include "physics/PhysicsDebugDraw.h"

namespace DemolisherWeapon {

	class PhysicsDebugDrawRender :
		public IRander
	{
	public:
		PhysicsDebugDrawRender();
		~PhysicsDebugDrawRender();

		void Release();

		void Render()override;

		//デバッグ表示のモードを設定する
		void SetDebugMode(int debugMode)
		{
#ifndef DW_DX12_TEMPORARY
			m_enable = true;
			Init();
			m_physicsDrawer.setDebugMode(debugMode);
#endif
		}

		//デバッグ表示が有効か？
		bool IsEnable()const { return m_enable; }

	private:
		void Init();

		bool m_enable = false;
		bool m_isInit = false;
		PhysicsDebugDraw m_physicsDrawer;
	};

}
