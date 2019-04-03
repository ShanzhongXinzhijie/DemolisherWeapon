#pragma once

namespace DemolisherWeapon {

	class GaussianBlurRender{
	public:
		GaussianBlurRender();
		~GaussianBlurRender();

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="souce">ぼかすシェーダーリソースビュー</param>
		/// <param name="dispersion">ぼかしの強さ</param>
		void Init(ID3D11ShaderResourceView*& souce, float dispersion);

		void Release();

		/// <summary>
		/// ガウスブラーをかける
		/// </summary>
		void Blur();

		/// <summary>
		/// シェーダーリソースビューの取得
		/// </summary>
		/// <returns></returns>
		ID3D11ShaderResourceView*& GetSRV() {
			return m_outputYSRV;
		}

	private:
		Shader m_vsx, m_vsy, m_ps;

		//入力テクスチャ
		ID3D11ShaderResourceView* m_souce = nullptr;
		//出力テクスチャ
		ID3D11Texture2D*			m_outputX = nullptr;
		ID3D11RenderTargetView*		m_outputXRTV = nullptr;
		ID3D11ShaderResourceView*	m_outputXSRV = nullptr;

		ID3D11Texture2D*			m_outputY = nullptr;
		ID3D11RenderTargetView*		m_outputYRTV = nullptr;
		ID3D11ShaderResourceView*	m_outputYSRV = nullptr;

		D3D11_TEXTURE2D_DESC m_texDesc;

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

		D3D11_VIEWPORT m_viewport;//ビューポート
	};

}
