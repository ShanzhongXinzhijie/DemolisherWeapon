#pragma once
#include "IRander.h"
#include "DirectXTK/Inc/PrimitiveBatch.h"

namespace DemolisherWeapon {

	class PrimitiveRender :
		public IRander
	{
	public:
		PrimitiveRender();
		~PrimitiveRender();

		void Init();

		void Render()override;
		void PostRender()override;

		void AddLine(const CVector3& start, const CVector3& end, const CVector4& color) {
			m_isDraw = true;
			m_ilneList.emplace_back(start, end, color);
		}

	private:
		std::unique_ptr<DirectX::BasicEffect> m_effect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		bool m_isDraw = false;

		struct Line {
			Line(const CVector3& s, const CVector3& e, const CVector4& c) : start(s), end(e), color(c) {};
			CVector3 start;
			CVector3 end;
			CVector4 color;
		};
		std::list<Line> m_ilneList;
	};

}