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

		void Resize()override;

		//3D描画
		void Render()override;
		void PostRender()override;

		//2D描画
		void Render2D();
		void PostRender2D();
		//HUD描画
		void RenderHUD(int HUDNum);
		void PostRenderHUD();

		/// <summary>
		/// 線分を描画する
		/// </summary>
		/// <param name="start">始点</param>
		/// <param name="end">終点</param>
		/// <param name="color">色</param>
		/// <param name="is3D">3D描画か？(falseで2D描画)</param>
		/// <param name="isHUD">HUDに描画するか?(0以上でその番号のHUDに描画)(HUD描画は2D描画)</param>
		void AddLine(const CVector3& start, const CVector3& end, const CVector4& color, bool is3D, int isHUD) {
			if (isHUD >= 0) {
				m_isDrawHUD[isHUD] = true;
				if (m_ilneListHUD.size() <= isHUD) { m_ilneListHUD.resize(isHUD + 1); }
				m_ilneListHUD[isHUD].emplace_back(start*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), end*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), color);
				return;
			}
			if (is3D) {
				m_isDraw3D = true;
				m_ilneList3D.emplace_back(start, end, color);
			}
			else {
				m_isDraw2D = true;
				m_ilneList2D.emplace_back(start*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), end*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), color);
			}
		}
		/// <summary>
		/// 四角形を描画する(2D)
		/// </summary>
		/// <param name="min">最小座標</param>
		/// <param name="max">最大座標</param>
		/// <param name="color">色</param>
		/// <param name="isHUD">HUDに描画するか?(0以上でその番号のHUDに描画)(HUD描画は2D描画)</param>
		void AddQuad(const CVector3& min, const CVector3& max, const CVector4& color, int isHUD) {
			if (isHUD >= 0) {
				m_isDrawHUD[isHUD] = true;
				if (m_quadListHUD.size() <= isHUD) { m_quadListHUD.resize(isHUD + 1); }
				m_quadListHUD[isHUD].emplace_back(min*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), max*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), color);
				return;
			}
			m_isDraw2D = true;
			m_quadList2D.emplace_back(min*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), max*CVector3(m_2dCamera.GetWidth(), -m_2dCamera.GetHeight(), 0.0f), color);
		}

	private:
		//PrimitiveBatch関係
		std::unique_ptr<DirectX::BasicEffect> m_effect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		//2D描画用カメラ
		GameObj::NoRegisterOrthoCamera m_2dCamera;

		//描画を実行するか?
		bool m_isDraw3D = false;
		bool m_isDraw2D = false;
		std::vector<bool> m_isDrawHUD;

		//プリミティブ
		struct Line {
			Line(const CVector3& s, const CVector3& e, const CVector4& c) : start(s), end(e), color(c) {};
			CVector3 start;
			CVector3 end;
			CVector4 color;
		};
		//線分
		std::list<Line> m_ilneList3D, m_ilneList2D;
		std::vector<std::list<Line>> m_ilneListHUD;
		//四角形
		std::list<Line> m_quadList2D;
		std::vector<std::list<Line>> m_quadListHUD;
	};

}