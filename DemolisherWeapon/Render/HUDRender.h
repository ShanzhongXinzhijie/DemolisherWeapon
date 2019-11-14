#pragma once
#include "IRander.h"

namespace DemolisherWeapon {
	class HUDRender :
		public IRander
	{
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="screen_min">描画範囲(最小側)</param>
		/// <param name="screen_max">描画範囲(最大側)</param>
		/// <param name="screenSize">テクスチャサイズ</param>
		void Init(int HUDNum, const CVector2& screen_min, const CVector2& screen_max, const CVector2& screenSize);
		
		/// <summary>
		/// レンダー処理
		/// </summary>
		void Render();

	private:
		int m_HUDNum = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_texture;	//テクスチャ
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_RTV;		//レンダーターゲットビュー
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;		//SRV

		//バックバッファへの描画用
		Shader m_vs, m_ps;
		CPrimitive m_drawSpace;
		CPrimitive::SVertex m_vertex[4];
		unsigned long m_index[4] = { 0,1,2,3 };
	};
}
