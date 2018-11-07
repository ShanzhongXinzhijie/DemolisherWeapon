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

		void Init();
		void Release();

		void Render()override;

		//デバッグ表示のモードを設定する
		void SetDebugMode(int debugMode)
		{
			m_physicsDrawer.setDebugMode(debugMode);
		}

	private:
		PhysicsDebugDraw m_physicsDrawer;
	};

}
