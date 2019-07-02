#include "DWstdafx.h"
#include "DepthOfFieldRender.h"

namespace DemolisherWeapon {

	DepthOfFieldRender::DepthOfFieldRender()
	{
	}
	DepthOfFieldRender::~DepthOfFieldRender()
	{
		Release();
	}

	void DepthOfFieldRender::Init(float texScale) {
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//合成用サンプラー
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		ge.GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);

		//縮小テクスチャサイズ算出
		m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W() * texScale), m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H() * texScale);

		//縮小テクスチャ作成
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = m_textureSizeX;
		texDesc.Height = m_textureSizeY;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_tex);
		ge.GetD3DDevice()->CreateShaderResourceView(m_tex, nullptr, &m_SRV);//シェーダーリソースビュー
		ge.GetD3DDevice()->CreateRenderTargetView(m_tex, nullptr, &m_RTV);//レンダーターゲット

		//ビューポート
		m_viewport.Width = (float)texDesc.Width;
		m_viewport.Height = (float)texDesc.Height;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//縮小描画用シェーダ
		m_vsResize.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		m_psResize.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

		//ガウスブラー
		//m_gaussBlur.Init(m_SRV, 0.5f);

		//合成シェーダ
		m_vsCompo.Load("Preset/shader/DOF.fx", "VSMain", Shader::EnType::VS);
		m_psCompo.Load("Preset/shader/DOF.fx", "PSMain", Shader::EnType::PS);
		//定数バッファ
		ShaderUtil::CreateConstantBuffer(sizeof(SPSConstantBuffer), &m_cb);
	}
	void DepthOfFieldRender::Release() {
		if (m_tex) { m_tex->Release(); m_tex = nullptr; }
		if (m_SRV) { m_SRV->Release(); m_SRV = nullptr; }
		if (m_RTV) { m_RTV->Release(); m_RTV = nullptr; }
		if (m_cb) { m_cb->Release(); m_cb = nullptr; }
		if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
	}

	void DepthOfFieldRender::Render() {
		if (!m_enable) { return; }

		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"Depth of field");

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//縮小
		{
			//描画先をクリア
			float clearColor[4] = {
				0.0f, 0.0f, 0.0f, 0.0f
			};
			rc->ClearRenderTargetView(m_RTV, clearColor);

			//ビューポート記録
			D3D11_VIEWPORT oldviewport; UINT kaz = 1;
			rc->RSGetViewports(&kaz, &oldviewport);
			//ビューポート設定
			rc->RSSetViewports(1, &m_viewport);

			//SRVをセット	
			rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());

			//描画先を縮小テクスチャにする
			rc->OMSetRenderTargets(1, &m_RTV, nullptr);

			//シェーダーを設定
			rc->VSSetShader((ID3D11VertexShader*)m_vsResize.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_psResize.GetBody(), NULL, 0);
			//入力レイアウトを設定。
			rc->IASetInputLayout(m_vsResize.GetInputLayout());
			//サンプラステートを設定。
			rc->PSSetSamplers(0, 1, &m_samplerState);

			//描画
			GetEngine().GetGraphicsEngine().DrawFullScreen();

			//SRVを解除
			ID3D11ShaderResourceView* view[] = {
						NULL
			};
			rc->PSSetShaderResources(0, 1, view);

			//レンダーターゲット解除
			rc->OMSetRenderTargets(0, NULL, NULL);

			//ビューポート戻す
			rc->RSSetViewports(1, &oldviewport);
		}

		//ぼかし
		//m_gaussBlur.Blur();

		//合成
		{
			//SRVをセット	
			rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
			rc->PSSetShaderResources(1, 1, &m_SRV);// m_gaussBlur.GetSRV());
			rc->PSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));

			//最終レンダーターゲットのスワップ
			GetEngine().GetGraphicsEngine().GetFRT().Swap();

			//描画先を最終レンダーターゲットにする
			GetEngine().GetGraphicsEngine().SetFinalRenderTarget_NonDepth();

			//シェーダーを設定
			rc->VSSetShader((ID3D11VertexShader*)m_vsCompo.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_psCompo.GetBody(), NULL, 0);
			//入力レイアウトを設定。
			rc->IASetInputLayout(m_vsCompo.GetInputLayout());
			//サンプラステートを設定。
			rc->PSSetSamplers(0, 1, &m_samplerState);

			//定数バッファ
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_cbSetting, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);

			//描画
			GetEngine().GetGraphicsEngine().DrawFullScreen();

			//SRVを解除
			ID3D11ShaderResourceView* view[] = {
						NULL
			};
			rc->PSSetShaderResources(0, 1, view);
			rc->PSSetShaderResources(1, 1, view);
			rc->PSSetShaderResources(2, 1, view);

			//レンダーターゲット解除
			rc->OMSetRenderTargets(0, NULL, NULL);
		}

		//GPUイベントの終了
		GetGraphicsEngine().EndGPUEvent();
	}

}