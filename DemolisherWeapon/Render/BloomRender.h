#pragma once
#include "IRander.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {
	class BloomRender : public IRander
	{
	public:
		BloomRender();
		~BloomRender();

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="texScale">texScale*3Dフレームバッファサイズ = テクスチャサイズ</param>
		void Init(float texScale = 0.5f);
		void Release();

		void Render() override;

		//シェーダーリソースビューの取得
		ID3D11ShaderResourceView*& GetSRV() {
			return m_SRV;
		}

		//有効無効の設定
		void SetEnable(bool enable) { m_enable = enable; }
		bool GetEnable()const { return m_enable; }

		//輝度しきい値の設定
		void SetLuminanceThreshold(float f) { m_luminanceThreshold = f; }
		float GetLuminanceThreshold()const { return m_luminanceThreshold; }

	private:
		bool m_enable = true;

		Shader m_cs;
		ID3D11Texture2D* m_tex = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
		ID3D11RenderTargetView* m_RTV = nullptr;
		UINT m_textureSizeX = 0, m_textureSizeY = 0;

		//定数バッファ
		//[Bloom.fx:CSCb]
		struct SCSConstantBuffer {
			//フレームバッファ解像度
			unsigned int win_x;
			unsigned int win_y;
			//出力テクスチャ解像度
			unsigned int out_x;
			unsigned int out_y;

			//輝度しきい値
			float luminanceThreshold;
		};
		ID3D11Buffer* m_cb = nullptr;//輝度しきい値

		//設定
		float m_luminanceThreshold = 1.0f;

		//干渉縞テクスチャ
		ID3D11ShaderResourceView* m_interferenceFringesSRV = nullptr;

		//ガウスブラーレンダー
		GaussianBlurRender m_gaussBlur;

		//合成用
		ID3D11BlendState* m_blendState = nullptr;
		Shader m_vs, m_ps;
		ID3D11SamplerState* m_samplerState = nullptr;
	};
}
