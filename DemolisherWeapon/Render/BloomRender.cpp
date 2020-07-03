#include "DWstdafx.h"
#include "BloomRender.h"

namespace DemolisherWeapon {
	void BloomRender::Init(float texScale)
	{
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//コンピュートシェーダ
		m_cs.Load("Preset/shader/Bloom.fx", "CSmain", Shader::EnType::CS);

		//テクスチャサイズ算出
		m_texScale = texScale;
		m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W() * texScale), m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H() * texScale);

		//出力テクスチャ作成
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
		texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_tex);
		ge.GetD3DDevice()->CreateShaderResourceView(m_tex, nullptr, &m_SRV);//シェーダーリソースビュー
		ge.GetD3DDevice()->CreateRenderTargetView(m_tex, nullptr, &m_RTV);//レンダーターゲット

		//OutPutUAV
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC	uavDesc;
			ZeroMemory(&uavDesc, sizeof(uavDesc));

			uavDesc.Format = texDesc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			HRESULT	hr;
			hr = ge.GetD3DDevice()->CreateUnorderedAccessView(m_tex, &uavDesc, &m_outputUAV);
		}

		//定数バッファ
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = (((sizeof(SCSConstantBuffer) - 1) / 16) + 1) * 16;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		ge.GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cb);

		//干渉縞テクスチャ
		HRESULT hr = DirectX::CreateDDSTextureFromFile(ge.GetD3DDevice(), L"Preset/sprite/Blackbody_Enumerated_6500K.dds", nullptr, &m_interferenceFringesSRV);
		
		//ガウスブラー
		m_gaussBlur.Init(m_SRV, 1.5f);

		//加算ブレンドステート
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_GREEN;
		ge.GetD3DDevice()->CreateBlendState(&blendDesc, &m_blendState);

		//合成用シェーダ
		m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

		//合成用サンプラー
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		ge.GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
	}

	void BloomRender::Release() {
		if (m_tex) { m_tex->Release(); m_tex = nullptr; }
		if (m_SRV) { m_SRV->Release(); m_SRV = nullptr; }
		if (m_RTV) { m_RTV->Release(); m_RTV = nullptr; }
		if (m_outputUAV) { m_outputUAV->Release(); m_outputUAV = nullptr; }
		if (m_cb) { m_cb->Release(); m_cb = nullptr; }
		if (m_interferenceFringesSRV) { m_interferenceFringesSRV->Release(); m_interferenceFringesSRV = nullptr; }
		if (m_blendState) { m_blendState->Release(); m_blendState = nullptr; }
		if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
	}

	void BloomRender::Resize() {
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//テクスチャサイズ算出
		m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W() * m_texScale);
		m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H() * m_texScale);

		//出力テクスチャ作成
		D3D11_TEXTURE2D_DESC texDesc;
		m_tex->GetDesc(&texDesc);
		texDesc.Width = m_textureSizeX;
		texDesc.Height = m_textureSizeY;
		m_tex->Release(); m_tex = nullptr;
		m_SRV->Release(); m_SRV = nullptr;
		m_RTV->Release(); m_RTV = nullptr;		
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_tex);
		ge.GetD3DDevice()->CreateShaderResourceView(m_tex, nullptr, &m_SRV);
		ge.GetD3DDevice()->CreateRenderTargetView(m_tex, nullptr, &m_RTV);

		//OutPutUAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		m_outputUAV->GetDesc(&uavDesc);
		m_outputUAV->Release(); m_outputUAV = nullptr;
		ge.GetD3DDevice()->CreateUnorderedAccessView(m_tex, &uavDesc, &m_outputUAV);		

		//ガウスブラー
		m_gaussBlur.ResetSource(m_SRV);
	}

	void BloomRender::Render()
	{
		if (!m_enable) { return; }

		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"BloomRender");

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//描画先をクリア
		float clearColor[4] = {
			0.0f, 0.0f, 0.0f, 0.0f
		};
		rc->ClearRenderTargetView(m_RTV, clearColor);		
		
		// 設定
		{
			//定数バッファ
			SCSConstantBuffer csCb;
			csCb.win_x = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
			csCb.win_y = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
			csCb.out_x = m_textureSizeX;
			csCb.out_y = m_textureSizeY;
			csCb.luminanceThreshold = m_luminanceThreshold;
			rc->UpdateSubresource(m_cb, 0, nullptr, &csCb, 0, 0);
			rc->CSSetConstantBuffers(0, 1, &m_cb);

			//CS
			rc->CSSetShader((ID3D11ComputeShader*)m_cs.GetBody(), NULL, 0);
			//UAV
			rc->CSSetUnorderedAccessViews(0, 1, &m_outputUAV, nullptr);

			//SRVを設定
			rc->CSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
			rc->CSSetShaderResources(2, 1, &m_interferenceFringesSRV);

			//サンプラステートを設定。
			rc->CSSetSamplers(0, 1, &m_samplerState);
		}

		// ディスパッチ
		rc->Dispatch((UINT)std::ceil(m_textureSizeX / 32.0f), (UINT)std::ceil(m_textureSizeY / 32.0f), 1);

		//設定解除
		{
			ID3D11Buffer* pCB = NULL;
			rc->CSSetConstantBuffers(0, 1, &pCB);

			ID3D11ShaderResourceView*	pReses = NULL;
			rc->CSSetShaderResources(1, 1, &pReses);
			rc->CSSetShaderResources(2, 1, &pReses);

			ID3D11UnorderedAccessView*	pUAV = NULL;
			rc->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);
		}

		//ぼかす
		m_gaussBlur.Blur();

		//合成

		//ブレンドステート変更
		ID3D11BlendState* oldBlendState = nullptr; FLOAT oldf[4]; UINT olduint;
		GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldf, &olduint);

		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);

		//SRVをセット	
		rc->PSSetShaderResources(0, 1, &m_gaussBlur.GetSRV());
		
		//描画先を最終レンダーターゲットにする
		GetEngine().GetGraphicsEngine().SetFinalRenderTarget_NonDepth();

		//シェーダーを設定
		rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
		rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
		//入力レイアウトを設定。
		rc->IASetInputLayout(m_vs.GetInputLayout());
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
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//ブレンドステート戻す
		if (oldBlendState) {
			GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldf, olduint);
			oldBlendState->Release();
		}

		//GPUイベントの終了
		GetGraphicsEngine().EndGPUEvent();
	}
}