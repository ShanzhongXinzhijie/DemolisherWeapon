#pragma once

namespace DemolisherWeapon {

	class GaussianBlurRender{
	public:
		GaussianBlurRender();
		~GaussianBlurRender();

		void Init(ID3D11ShaderResourceView*& souce);
		void Release();

		void Blur();

	private:
		Shader m_vsx, m_vsy, m_ps;

		//入力テクスチャ
		ID3D11ShaderResourceView* m_souce = nullptr;
		//出力テクスチャ
		ID3D11Texture2D*		m_outputX = nullptr;
		ID3D11RenderTargetView* m_outputXRTV = nullptr;
		ID3D11ShaderResourceView* m_outputXSRV = nullptr;
		ID3D11Texture2D*		m_outputY = nullptr;
		ID3D11RenderTargetView* m_outputYRTV = nullptr;
		ID3D11ShaderResourceView* m_outputYSRV = nullptr;

		//定数バッファ
		static const int NUM_WEIGHTS = 8; 
		struct SBlurParam {
			CVector4 offset;
			float weights[NUM_WEIGHTS];
		};
		SBlurParam m_blurParam;
		ID3D11Buffer* m_cb = nullptr;

		//サンプラー
		ID3D11SamplerState* m_samplerState = nullptr;
	};

}
