#include "DWstdafx.h"
#include "DX11Test.h"

namespace DemolisherWeapon {

	namespace {
		template<class T>
		void RELEASE(T*& t) {
			if (t) {
				t->Release();
				t = nullptr;
			}
		}
	}

	bool DX11Test::Init(HWND hWnd, const InitEngineParameter& initParam) {
		//リフレッシュレートを取得
		auto refleshRate = Util::GetRefreshRate(hWnd);

		//スワップチェインを作成するための情報を設定する。
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;									//スワップチェインのバッファ数。通常は１。
		sd.BufferDesc.Width = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W());	//フレームバッファの幅。
		sd.BufferDesc.Height = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H());	//フレームバッファの高さ。
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//フレームバッファのフォーマット。R8G8B8A8の32bit。
		sd.BufferDesc.RefreshRate.Numerator = refleshRate;//モニタのリフレッシュレート。(バックバッファとフロントバッファを入れ替えるタイミングとなる。)
		sd.BufferDesc.RefreshRate.Denominator = 1;			//２にしたら30fpsになる。1でいい。
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//サーフェスまたはリソースを出力レンダー ターゲットとして使用します。
		sd.OutputWindow = hWnd;								//出力先のウィンドウハンドル。
		sd.SampleDesc.Count = 1;							//1でいい。
		sd.SampleDesc.Quality = 0;							//MSAAなし。0でいい。
		sd.Windowed = initParam.isWindowMode ? TRUE : FALSE;//ウィンドウモード。TRUEでよい。

		//利用するDirectXの機能セット。
		//この配列はD3D11CreateDeviceAndSwapChainの引数として使う。
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
		};
		D3D_FEATURE_LEVEL confirmedFeatureLevel;

		//D3Dデバイスとスワップチェインを作成する。
		D3D11CreateDeviceAndSwapChain(
			NULL,											//NULLでいい。
			D3D_DRIVER_TYPE_HARDWARE,						//D3Dデバイスがアクセスするドライバーの種類。
															//基本的にD3D_DRIVER_TYPE_HARDWAREを指定すればよい。
			NULL,											//NULLでいい。
			0,												//０でいい。
			featureLevels,									//D3Dデバイスのターゲットとなる機能セットを指定する。
			sizeof(featureLevels) / sizeof(featureLevels[0]),	//機能セットの数。
			D3D11_SDK_VERSION,								//使用するDirectXのバージョン。
															//D3D11_SDK_VERSIONを指定すればよい。
			&sd,											//スワップチェインを作成するための情報。
			&m_pSwapChain,									//作成したスワップチェインのアドレスの格納先。
			&m_pd3dDevice,									//作成したD3Dデバイスのアドレスの格納先。
			&confirmedFeatureLevel,							//使用される機能セットの格納先。
			&m_pd3dDeviceContext							//作成したD3Dデバイスコンテキストのアドレスの格納先。
		);

		//書き込み先になるレンダリングターゲットを作成。
		ID3D11Texture2D* pBackBuffer = NULL;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_backBuffer);
		pBackBuffer->Release();
		//深度ステンシルビューの作成。
		{
			//深度テクスチャの作成。
			D3D11_TEXTURE2D_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(texDesc));
			texDesc.Width = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W());
			texDesc.Height = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H());
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_D32_FLOAT;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;
			m_pd3dDevice->CreateTexture2D(&texDesc, NULL, &m_depthStencil);
			//深度ステンシルビューを作成。
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			ZeroMemory(&descDSV, sizeof(descDSV));
			descDSV.Format = texDesc.Format;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;
			m_pd3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
		}

		//ラスタライザを初期化。
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		m_pd3dDevice->CreateRasterizerState(&desc, &m_rasterizerState);
		m_pd3dDeviceContext->RSSetState(m_rasterizerState);

		//デプスステンシルステート
		{
			D3D11_DEPTH_STENCIL_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			desc.StencilEnable = false;
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			GetD3DDevice()->CreateDepthStencilState(&desc, &m_depthStencilState);
			GetD3DDeviceContext()->OMSetDepthStencilState(m_depthStencilState, 0);
		}

		return true;
	}

	void DX11Test::Release() {
		RELEASE(m_depthStencilState);
		RELEASE(m_rasterizerState);
		RELEASE(m_depthStencil);
		RELEASE(m_depthStencilView);
		RELEASE(m_backBuffer);
		RELEASE(m_pSwapChain);
		RELEASE(m_pd3dDeviceContext);
		RELEASE(m_pd3dDevice);
	}

	void DX11Test::ChangeFrameBufferSize() {
		//※スワップチェインを参照しているオブジェクトを削除してないとm_pSwapChain->ResizeBuffersができない
		//バックバッファ死亡!!!死!!
		if (m_backBuffer) { m_backBuffer->Release(); m_backBuffer = NULL; }

		//スワップチェインサイズ変更
		DXGI_SWAP_CHAIN_DESC sd;
		m_pSwapChain->GetDesc(&sd);
		HRESULT hr = m_pSwapChain->ResizeBuffers(0, (UINT)GetGraphicsEngine().GetFrameBuffer_W(), (UINT)GetGraphicsEngine().GetFrameBuffer_H(), sd.BufferDesc.Format, sd.Flags);

		//バックバッファ再作成
		ID3D11Texture2D* pBackBuffer = NULL;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_backBuffer);
		pBackBuffer->Release();

		//深度ステンシルの設定コピー
		D3D11_TEXTURE2D_DESC texDesc;
		m_depthStencil->GetDesc(&texDesc);
		texDesc.Width = (UINT)GetGraphicsEngine().GetFrameBuffer_W();
		texDesc.Height = (UINT)GetGraphicsEngine().GetFrameBuffer_H();
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		m_depthStencilView->GetDesc(&descDSV);
		descDSV.Format = texDesc.Format;

		//深度ステンシル死亡!!!死!!
		if (m_depthStencil) { m_depthStencil->Release(); m_depthStencil = NULL; }
		if (m_depthStencilView) { m_depthStencilView->Release(); m_depthStencilView = NULL; }

		//深度テクスチャ再作成
		m_pd3dDevice->CreateTexture2D(&texDesc, NULL, &m_depthStencil);
		//深度ステンシルビュー再作成
		m_pd3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
	}

	void DX11Test::SetBackBufferToRenderTarget()
	{
		//描き込み先をバックバッファにする。
		m_pd3dDeviceContext->OMSetRenderTargets(1, &m_backBuffer, m_depthStencilView);
	}
	void DX11Test::SwapBackBuffer()
	{
		//バックバッファとフロントバッファを入れ替える。
		m_pSwapChain->Present(GetGraphicsEngine().GetUseVSync() ? 1 : 0, 0);
	}

}