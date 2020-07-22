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

		//�f�o�b�O�\���̃��[�h��ݒ肷��
		void SetDebugMode(int debugMode)
		{
#ifndef DW_DX12_TEMPORARY
			m_enable = true;
			Init();
			m_physicsDrawer.setDebugMode(debugMode);
#endif
		}

		//�f�o�b�O�\�����L�����H
		bool IsEnable()const { return m_enable; }

	private:
		void Init();

		bool m_enable = false;
		bool m_isInit = false;
		PhysicsDebugDraw m_physicsDrawer;
	};

}
