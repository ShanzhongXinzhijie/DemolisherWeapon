#pragma once
#include "bulletPhysics/src/LinearMath/btIDebugDraw.h"
#include <array> 
#include "Graphic/CPrimitive.h"

namespace DemolisherWeapon {

	class PhysicsDebugDraw :
		public btIDebugDraw
	{
		struct SConstantBuffer {
			CMatrix mView;
			CMatrix mProj;
		};
		int m_debugMode = btIDebugDraw::DBG_NoDebug;
		CPrimitive m_primitive;
		Shader m_vs;	//���_�V�F�[�_�[�B
		Shader m_ps;	//�s�N�Z���V�F�[�_�[�B
		ID3D11Buffer* m_cb = nullptr;
		static const int VERTEX_MAX = 1000000;
		std::array<CPrimitive::SVertex, VERTEX_MAX> m_vertexBuffer;

		int m_numLine = 0;
	public:
		~PhysicsDebugDraw();

		void Init();
		void Release();

		void BeginDraw()
		{
			m_numLine = 0;
		}

		void EndDraw();
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
		void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override
		{
		}
		void reportErrorWarning(const char*) override
		{
		}
		void draw3dText(const btVector3&, const char*) override
		{
		}
		void setDebugMode(int debugMode) override
		{
			m_debugMode = debugMode;
		}
		int	getDebugMode() const override
		{
			return m_debugMode;
		}
	};

}