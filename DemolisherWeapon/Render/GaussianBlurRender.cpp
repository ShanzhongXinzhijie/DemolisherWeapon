#include "DWstdafx.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {
	
	void GaussianBlurRender::Init(ID3D11ShaderResourceView*& souce, float dispersion, const CVector2& sampleScale) {

		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();
		
		m_souce = souce;

		//souceのテクスチャ取得
		ID3D11Resource* pResource = nullptr;
		ID3D11Texture2D* pTex = nullptr;
		souce->GetResource(&pResource);
		pTex = (ID3D11Texture2D*)pResource;
		
		//desc取得
		D3D11_TEXTURE2D_DESC texDesc;
		pTex->GetDesc(&texDesc);

		pResource->Release(); pResource = nullptr; pTex = nullptr;

		//出力テクスチャDESC
		ZeroMemory(&m_texDesc, sizeof(m_texDesc));
		m_texDesc.Width = texDesc.Width;
		m_texDesc.Height = texDesc.Height;
		m_texDesc.MipLevels = 1;
		m_texDesc.ArraySize = 1;
		m_texDesc.Format = texDesc.Format;
		m_texDesc.SampleDesc.Count = 1;
		m_texDesc.SampleDesc.Quality = 0;
		m_texDesc.Usage = D3D11_USAGE_DEFAULT;
		m_texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		m_texDesc.CPUAccessFlags = 0;
		m_texDesc.MiscFlags = 0;

		//サンプル間隔
		m_sampleScaleSetting = sampleScale;
		m_sampleScale = sampleScale;
		if (m_sampleScale.x < 0.0f) { m_sampleScale.x = (float)m_texDesc.Width; }
		if (m_sampleScale.y < 0.0f) { m_sampleScale.y = (float)m_texDesc.Height; }

		//出力テクスチャの作成
		ge.GetD3DDevice()->CreateTexture2D(&m_texDesc, NULL, &m_outputX);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputX, nullptr, &m_outputXRTV);//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputX, nullptr, &m_outputXSRV);//シェーダーリソースビュー

		ge.GetD3DDevice()->CreateTexture2D(&m_texDesc, NULL, &m_outputY);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputY, nullptr, &m_outputYRTV);//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputY, nullptr, &m_outputYSRV);//シェーダーリソースビュー

		//ビューポート
		m_viewport.Width = (float)texDesc.Width;
		m_viewport.Height = (float)texDesc.Height;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//シェーダーをロード。
		m_vsx.Load("Preset/shader/gaussianblur.fx", "VSXBlur", Shader::EnType::VS);
		m_vsy.Load("Preset/shader/gaussianblur.fx", "VSYBlur", Shader::EnType::VS);
		m_ps.Load("Preset/shader/gaussianblur.fx", "PSBlur", Shader::EnType::PS);

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

	void GaussianBlurRender::ResetSource(ID3D11ShaderResourceView*& souce) {
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		m_souce = souce;

		//souceのテクスチャ取得
		ID3D11Resource* pResource = nullptr;
		ID3D11Texture2D* pTex = nullptr;
		souce->GetResource(&pResource);
		pTex = (ID3D11Texture2D*)pResource;

		//desc取得
		D3D11_TEXTURE2D_DESC texDesc;
		pTex->GetDesc(&texDesc);

		pResource->Release(); pResource = nullptr; pTex = nullptr;
		
		//出力テクスチャDESC
		m_texDesc.Width = texDesc.Width;
		m_texDesc.Height = texDesc.Height;
		m_texDesc.Format = texDesc.Format;

		//サンプル間隔
		m_sampleScale = m_sampleScaleSetting;
		if (m_sampleScale.x < 0.0f) { m_sampleScale.x = (float)m_texDesc.Width; }
		if (m_sampleScale.y < 0.0f) { m_sampleScale.y = (float)m_texDesc.Height; }

		//出力テクスチャの作成
		m_outputX->Release(); m_outputX = nullptr;
		m_outputXRTV->Release(); m_outputXRTV = nullptr;
		m_outputXSRV->Release(); m_outputXSRV = nullptr;
		ge.GetD3DDevice()->CreateTexture2D(&m_texDesc, NULL, &m_outputX);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputX, nullptr, &m_outputXRTV);//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputX, nullptr, &m_outputXSRV);//シェーダーリソースビュー
		m_outputY->Release(); m_outputY = nullptr;
		m_outputYRTV->Release(); m_outputYRTV = nullptr;
		m_outputYSRV->Release(); m_outputYSRV = nullptr;
		ge.GetD3DDevice()->CreateTexture2D(&m_texDesc, NULL, &m_outputY);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputY, nullptr, &m_outputYRTV);//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputY, nullptr, &m_outputYSRV);//シェーダーリソースビュー

		//ビューポート
		m_viewport.Width = (float)texDesc.Width;
		m_viewport.Height = (float)texDesc.Height;
	}

	void GaussianBlurRender::Blur() {

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//サンプラステートを設定。
		rc->PSSetSamplers(0, 1, &m_samplerState);

		//ビューポート記録
		D3D11_VIEWPORT oldviewport; UINT kaz = 1;
		rc->RSGetViewports(&kaz, &oldviewport);

		//ビューポート設定
		rc->RSSetViewports(1, &m_viewport);

		//Clear
		float clearcolor[4] = {};
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_outputXRTV, clearcolor);
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_outputYRTV, clearcolor);
		
		//XBlur

			//レンダーターゲットの設定
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_outputXRTV, nullptr);

			//シェーダーリソース
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(0, 1, &m_souce);
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_souce);

			//定数バッファ
			m_blurParam.offset.x = 16.0f / m_sampleScale.x;
			m_blurParam.offset.y = 0.0f;
			m_blurParam.sampleScale = m_sampleScale.x;
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_blurParam, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);
			rc->VSSetConstantBuffers(0, 1, &m_cb);

			//シェーダーを設定
			rc->VSSetShader((ID3D11VertexShader*)m_vsx.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
			//入力レイアウトを設定。
			rc->IASetInputLayout(m_vsx.GetInputLayout());

			//描画
			GetEngine().GetGraphicsEngine().DrawFullScreen();

		//YBlur

			//レンダーターゲットの設定
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_outputYRTV, nullptr);

			//シェーダーリソース
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(0, 1, &m_outputXSRV);
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_outputXSRV);

			//定数バッファ
			m_blurParam.offset.x = 0.0f;
			m_blurParam.offset.y = 16.0f / m_sampleScale.y;
			m_blurParam.sampleScale = m_sampleScale.y;
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_blurParam, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);
			rc->VSSetConstantBuffers(0, 1, &m_cb);

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

		//ビューポート戻す
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);
	}

}