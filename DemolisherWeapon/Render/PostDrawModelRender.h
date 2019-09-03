#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class PostDrawModelRender :
		public IRander
	{
	public:
		PostDrawModelRender() = default;

		void Init();

		void Render()override;
		void PostRender()override;

		enum enBlendMode {
			enAlpha,enAdd,
		};

		/// <summary>
		/// 描画するモデルを追加
		/// </summary>
		/// <param name="sm">描画するモデル</param>
		/// <param name="priority">描画順(数値が大きいほど後)(0～DRAW_PRIORITY_MAX - 1)</param>
		/// <param name="blendmode">ブレンドモード</param>
		void AddDrawModel(SkinModel* sm, int priority, enBlendMode blendmode = enAlpha, bool reverse = false) {
			if (blendmode == enAdd) {
				m_drawModelList_Add[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back( sm,reverse );
			}
			if (blendmode == enAlpha) {
				m_drawModelList_Alpha[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back( sm,reverse );
			}
		};

	private:
		//描画するモデルのリスト
		std::list<std::pair<SkinModel*, bool>> m_drawModelList_Alpha[DRAW_PRIORITY_MAX], m_drawModelList_Add[DRAW_PRIORITY_MAX];
		//ブレンドステート
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaBlendState, m_addBlendState;
		//デプスステンシルステート
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	};

}