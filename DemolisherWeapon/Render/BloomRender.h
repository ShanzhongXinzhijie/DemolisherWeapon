#pragma once
#include "IRander.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {
	class BloomRender : public IRander
	{
	public:
		BloomRender();
		~BloomRender();

		void Init();
		void Release();

		void Render() override;

		//シェーダーリソースビューの取得
		ID3D11ShaderResourceView*& GetSRV() {
			return m_SRV;
		}

		//有効無効の設定
		void SetEnable(bool enable) { m_enable = enable; }
		bool GetEnable()const { return m_enable; }

	private:
		bool m_enable = true;

		Shader m_cs;
		ID3D11Texture2D* m_tex = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
		ID3D11RenderTargetView* m_RTV = nullptr;

		//定数バッファ
		//[Bloom.fx:CSCb]
		struct SCSConstantBuffer {
			//フレームバッファ解像度
			unsigned int win_x;
			unsigned int win_y;

			//輝度しきい値
			float luminanceThreshold;
		};
		ID3D11Buffer* m_cb = nullptr;

		GaussianBlurRender m_gaussBlur;

		//合成用
		ID3D11BlendState* m_blendState = nullptr;
		Shader m_vs, m_ps;
		ID3D11SamplerState* m_samplerState = nullptr;
	};
}
