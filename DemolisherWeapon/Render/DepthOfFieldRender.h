#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class DepthOfFieldRender :
		public IRander
	{
	private:
		//定数バッファ
		//[DOF.fx:PSCb]
		struct SPSConstantBuffer {
			float focus = 0.125f;
			float focusArea = 0.1f;
			float Near = 0.0f;
			float Far = 0.5f;
			
			//カメラ設定
			float camNear = 1.0f;
			float camFar = 100.0f;
		};

	public:
		DepthOfFieldRender();
		~DepthOfFieldRender();

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="texScale">texScale*3Dフレームバッファサイズ = 縮小テクスチャサイズ</param>
		void Init(float texScale = 0.5f);
		void Release();

		void Render() override;

		//有効無効の設定
		void SetEnable(bool enable) { m_enable = enable; }
		bool GetEnable()const { return m_enable; }

		//焦点距離の設定
		void SetFocus(float focus) { m_cbSetting.focus = focus; }
		void SetFocusArea(float area) { m_cbSetting.focusArea = area; }//焦点の合う範囲
		void SetNear(float Near) { m_cbSetting.Near = Near; }
		void SetFar(float Far) { m_cbSetting.Far = Far; }

	private:
		bool m_enable = true;

		//サンプラー
		ID3D11SamplerState* m_samplerState = nullptr;

		//縮小テクスチャ
		ID3D11Texture2D* m_tex = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11RenderTargetView* m_RTV = nullptr;
		UINT m_textureSizeX = 0, m_textureSizeY = 0;
		Shader m_vsResize, m_psResize;
		D3D11_VIEWPORT m_viewport;//ビューポート

		//ガウスブラーレンダー
		GaussianBlurRender m_gaussBlur;

		//合成シェーダ
		Shader m_vsCompo, m_psCompo;

		//定数バッファ
		SPSConstantBuffer m_cbSetting;
		ID3D11Buffer* m_cb = nullptr;
	};

}