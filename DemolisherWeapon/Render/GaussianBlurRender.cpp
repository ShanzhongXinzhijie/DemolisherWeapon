#include "DWstdafx.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {

	GaussianBlurRender::GaussianBlurRender()
	{
	}
	GaussianBlurRender::~GaussianBlurRender()
	{
		Release();
	}

	void GaussianBlurRender::Init(ID3D11ShaderResourceView*& souce) {

		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		m_souce = souce;

		//souceのテクスチャ取得
		ID3D11Resource* pResource = nullptr;
		ID3D11Texture2D* pTex = nullptr;
		souce->GetResource(&pResource);
		pTex = (ID3D11Texture2D*)pResource;
		pResource->Release();
		//desc取得
		D3D11_TEXTURE2D_DESC texDesc;
		pTex->GetDesc(&texDesc);

		//出力テクスチャの作成
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_outputX);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputX, nullptr, &m_outputXRTV);//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputX, nullptr, &m_outputXSRV);//シェーダーリソースビュー

		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_outputY);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputX, nullptr, &m_outputYRTV);//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputX, nullptr, &m_outputYSRV);//シェーダーリソースビュー

		//シェーダーをロード。
		m_vsx.Load("shader/gaussianblur.fx", "VSXBlur", Shader::EnType::VS);
		m_vsy.Load("shader/gaussianblur.fx", "VSYBlur", Shader::EnType::VS);
		m_ps.Load("shader/gaussianblur.fx", "PSBlur", Shader::EnType::PS);

		//サンプラー
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_samplerState);

		//定数バッファ
		ShaderUtil::CreateConstantBuffer(sizeof(SBlurParam), &m_cb);

		//ガウスの重み
		float dispersion = 5.0f;//ぼかしの強さ?
		float total = 0;
		for (int i = 0; i < NUM_WEIGHTS; i++) {
			m_blurParam.weights[i] = expf(-0.5f*(float)(i*i) / dispersion);
			total += 2.0f*m_blurParam.weights[i];

		}
		// 規格化
		for (int i = 0; i < NUM_WEIGHTS; i++) {
			m_blurParam.weights[i] /= total;
		}
	}
	void GaussianBlurRender::Release() {
		m_outputX->Release();
		m_outputXRTV->Release();
		m_outputXSRV->Release();
		m_outputY->Release();
		m_outputYRTV->Release();
		m_outputYSRV->Release();

		m_cb->Release();
		m_samplerState->Release();
	}

	void GaussianBlurRender::Blur() {

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//サンプラステートを設定。
		rc->PSSetSamplers(0, 1, &m_samplerState);

		//Clear

		//XBlur

			//レンダーターゲットの設定
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_outputXRTV, nullptr);

			//シェーダーリソース
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_souce);

			//定数バッファ
			//m_blurParam.offset.x = 16.0f / m_srcTextureWidth;
			m_blurParam.offset.y = 0.0f;
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_blurParam, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);

			//シェーダーを設定
			rc->VSSetShader((ID3D11VertexShader*)m_vsx.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
			//入力レイアウトを設定。
			rc->IASetInputLayout(m_vsx.GetInputLayout());

			//描画
			GetEngine().GetGraphicsEngine().DrawFullScreen();

		//YBlur

			//レンダーターゲットの設定
			//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &outputFin, nullptr);

			//シェーダーリソース
			//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &outputX);

			//定数バッファ
			m_blurParam.offset.x = 0.0f;
		//	m_blurParam.offset.y = 16.0f / m_srcTextureHeight;
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_blurParam, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);

			//シェーダーを設定
			rc->VSSetShader((ID3D11VertexShader*)m_vsy.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
			//入力レイアウトを設定。
			rc->IASetInputLayout(m_vsy.GetInputLayout());

			//描画
			GetEngine().GetGraphicsEngine().DrawFullScreen();

		//SRVを解除
		ID3D11ShaderResourceView* view[] = {
					NULL
		};
		rc->PSSetShaderResources(0, 1, view);

		//レンダーターゲット解除
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
	}

}