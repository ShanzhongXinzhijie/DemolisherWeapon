#pragma once
#include "IRander.h"
#include "DirectXTK/Inc/PrimitiveBatch.h"

namespace DemolisherWeapon {

	class PrimitiveRender :
		public IRander
	{
	public:
		PrimitiveRender() = default;

		void Init();

		//3D�`��
		void Render()override;
		void PostRender()override;

		//2D�`��
		void Render2D();
		void PostRender2D();

		/// <summary>
		/// ������`�悷��
		/// </summary>
		/// <param name="start">�n�_</param>
		/// <param name="end">�I�_</param>
		/// <param name="color">�F</param>
		/// <param name="is3D">3D�`�悩�H(false��2D�`��)</param>
		void AddLine(const CVector3& start, const CVector3& end, const CVector4& color, bool is3D) {
			if (is3D) {
				m_isDraw3D = true;
				m_ilneList3D.emplace_back(start, end, color);
			}
			else {
				m_isDraw2D = true;
				m_ilneList2D.emplace_back(start*CVector3(m_2dCamera.GetWidth(), m_2dCamera.GetHeight(), 0.0f), end*CVector3(m_2dCamera.GetWidth(), m_2dCamera.GetHeight(), 0.0f), color);
			}
		}
		/// <summary>
		/// �l�p�`��`�悷��(2D)
		/// </summary>
		/// <param name="min">�ŏ����W</param>
		/// <param name="max">�ő���W</param>
		/// <param name="color">�F</param>
		void AddQuad(const CVector3& min, const CVector3& max, const CVector4& color) {			
			m_isDraw2D = true;
			m_quadList2D.emplace_back(min*CVector3(m_2dCamera.GetWidth(), m_2dCamera.GetHeight(), 0.0f), max*CVector3(m_2dCamera.GetWidth(), m_2dCamera.GetHeight(), 0.0f), color);
		}

	private:
		std::unique_ptr<DirectX::BasicEffect> m_effect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		GameObj::NoRegisterOrthoCamera m_2dCamera;

		//�`������s���邩?
		bool m_isDraw3D = false;
		bool m_isDraw2D = false;

		//����
		struct Line {
			Line(const CVector3& s, const CVector3& e, const CVector4& c) : start(s), end(e), color(c) {};
			CVector3 start;
			CVector3 end;
			CVector4 color;
		};
		std::list<Line> m_ilneList3D, m_ilneList2D;

		//�l�p�`
		std::list<Line> m_quadList2D;
	};

}