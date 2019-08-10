#pragma once

namespace DemolisherWeapon {

	class GaussianBlurRender{
	public:
		GaussianBlurRender() = default;
		~GaussianBlurRender() {
			Release();
		}

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="souce">ぼかすシェーダーリソースビュー</param>
		/// <param name="dispersion">ぼかしの強さ</param>
		/// <param name="sampleScale">サンプル間隔(-1で1テクセル間隔)(解像度640で1テクセル間隔なら、sampleScale = 640.0f)</param>
		void Init(ID3D11ShaderResourceView*& souce, float dispersion, const CVector2& sampleScale = {-1.0f,-1.0f});

		/// <summary>
		/// 解放
		/// </summary>
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
		Shader m_vsx, m_vsy, m_ps;//シェーダ

		//入力テクスチャ
		ID3D11ShaderResourceView* m_souce = nullptr;
		//出力テクスチャ
		ID3D11Texture2D*			m_outputX = nullptr;
		ID3D11RenderTargetView*		m_outputXRTV = nullptr;
		ID3D11ShaderResourceView*	m_outputXSRV = nullptr;

		ID3D11Texture2D*			m_outputY = nullptr;
		ID3D11RenderTargetView*		m_outputYRTV = nullptr;
		ID3D11ShaderResourceView*	m_outputYSRV = nullptr;

		D3D11_TEXTURE2D_DESC m_texDesc;//テクスチャ情報

		//定数バッファ
		//[gaussianblur.fx : CBBlur]
		static const int NUM_WEIGHTS = 8; 
		struct SBlurParam {
			CVector4 offset;
			float weights[NUM_WEIGHTS];
			float sampleScale;
		};
		SBlurParam m_blurParam;
		ID3D11Buffer* m_cb = nullptr;
		CVector2 m_sampleScale = { -1.0f,-1.0f };

		//サンプラー
		ID3D11SamplerState* m_samplerState = nullptr;

		D3D11_VIEWPORT m_viewport;//ビューポート
	};

}
