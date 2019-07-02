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

		/// <summary>
		/// 描画するモデルを追加
		/// </summary>
		/// <param name="sm">描画するモデル</param>
		/// <param name="priority">描画順(数値が大きいほど後)(0～DRAW_PRIORITY_MAX - 1)</param>
		/// <param name="isAdd">加算ブレンドか？</param>
		void AddDrawModel(SkinModel* sm, int priority, bool isAdd = false) {
			if (isAdd) {
				m_drawModelList_Add.emplace_back(sm);
			}
			else {
				m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back(sm);
			}
		};

	private:
		//描画するモデルのリスト
		std::list<SkinModel*> m_drawModelList[DRAW_PRIORITY_MAX], m_drawModelList_Add;
		//加算ブレンドステート
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_addBlendState;
	};

}