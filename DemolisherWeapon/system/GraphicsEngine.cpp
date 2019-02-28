#include "DWstdafx.h"
#include "GraphicsEngine.h"

namespace DemolisherWeapon {

GraphicsEngine::GraphicsEngine()
{
	
}
GraphicsEngine::~GraphicsEngine()
{
	Release();
}


void GraphicsEngine::ClearBackBuffer()
{
	//バックバッファを灰色で塗りつぶす。
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	m_pd3dDeviceContext->ClearRenderTargetView(m_backBuffer, ClearColor);
	m_pd3dDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);	
}
void GraphicsEngine::SetBackBufferToRenderTarget()
{
	//描き込み先をバックバッファにする。
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_backBuffer, m_depthStencilView);
}
void GraphicsEngine::SwapBackBuffer()
{
	//バックバッファとフロントバッファを入れ替える。
	m_pSwapChain->Present(m_useVSync?1:0, 0);
}
void GraphicsEngine::RunRenderManager() {
	m_renderManager.Render();
}

void GraphicsEngine::Release()
{
	if (m_depthStencilState) {
		m_depthStencilState->Release();
		m_depthStencilState = nullptr;
	}
	if (m_rasterizerState != NULL) {
		m_rasterizerState->Release();
		m_rasterizerState = NULL;
	}
	if (m_depthStencil != NULL) {
		m_depthStencil->Release();
		m_depthStencil = NULL;
	}
	if (m_depthStencilView != NULL) {
		m_depthStencilView->Release();
		m_depthStencilView = NULL;
	}
	if (m_backBuffer != NULL) {
		m_backBuffer->Release();
		m_backBuffer = NULL;
	}
	if (m_pSwapChain != NULL) {
		m_pSwapChain->Release();
		m_pSwapChain = NULL;
	}
	if (m_pd3dDeviceContext != NULL) {
		m_pd3dDeviceContext->Release();
		m_pd3dDeviceContext = NULL;
	}
	if (m_pd3dDevice != NULL) {
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}

	m_FRT.Release();
}
void GraphicsEngine::Init(HWND hWnd, const InitEngineParameter& initParam)
{
	FRAME_BUFFER_W = (float)initParam.frameBufferWidth;
	FRAME_BUFFER_H = (float)initParam.frameBufferHeight;
	
	//スワップチェインを作成するための情報を設定する。
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;									//スワップチェインのバッファ数。通常は１。
	sd.BufferDesc.Width = (UINT)FRAME_BUFFER_W;			//フレームバッファの幅。
	sd.BufferDesc.Height = (UINT)FRAME_BUFFER_H;		//フレームバッファの高さ。
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//フレームバッファのフォーマット。R8G8B8A8の32bit。
	sd.BufferDesc.RefreshRate.Numerator = initParam.refleshRate;	//モニタのリフレッシュレート。(バックバッファとフロントバッファを入れ替えるタイミングとなる。)
	sd.BufferDesc.RefreshRate.Denominator = 1;			//２にしたら30fpsになる。1でいい。
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//サーフェスまたはリソースを出力レンダー ターゲットとして使用します。
	sd.OutputWindow = hWnd;								//出力先のウィンドウハンドル。
	sd.SampleDesc.Count = 1;							//1でいい。
	sd.SampleDesc.Quality = 0;							//MSAAなし。0でいい。
	sd.Windowed = initParam.isWindowMode ? TRUE : FALSE;			//ウィンドウモード。TRUEでよい。

	//利用するDirectXの機能セット。
	//この配列はD3D11CreateDeviceAndSwapChainの引数として使う。
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		//D3D_FEATURE_LEVEL_10_1,
		//D3D_FEATURE_LEVEL_10_0,
	};

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
		&m_featureLevel,								//使用される機能セットの格納先。
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
		texDesc.Width = (UINT)FRAME_BUFFER_W;
		texDesc.Height = (UINT)FRAME_BUFFER_H;
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

	//ビューポートを初期化。
	SetViewport(0.0f, 0.0f, FRAME_BUFFER_W, FRAME_BUFFER_H);

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

		m_pd3dDevice->CreateDepthStencilState(&desc, &m_depthStencilState);
		m_pd3dDeviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}

	//画面分割用の比率に
	FRAME_BUFFER_3D_W = (float)initParam.frameBufferWidth3D;
	FRAME_BUFFER_3D_H = (float)initParam.frameBufferHeight3D;
	if (initParam.isSplitScreen == enVertical_TwoSplit) {
		FRAME_BUFFER_3D_H *= 0.5f;
	}
	if (initParam.isSplitScreen == enSide_TwoSplit) {
		FRAME_BUFFER_3D_W *= 0.5f;
	}

	//Sprite初期化
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(m_pd3dDevice, L"Preset/Font/myfile.spritefont");
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pd3dDeviceContext);
	m_spriteBatchPMA = std::make_unique<DirectX::SpriteBatch>(m_pd3dDeviceContext);

	//フルスクリーン描画プリミティブ初期化
	CPrimitive::SVertex vertex[4] = {
		{
			{-1.0f, -1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		},
		{
			{1.0f, -1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		},
		{
			{-1.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		},
		{
			{1.0f, 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		},
	};
	int index[4] = { 0,1,2,3 };
	m_fullscreen.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, 4, index);

	//ライトマネージャー
	m_lightManager.Init();

	//最終レンダーターゲット
	m_FRT.Init();

	//レンダー初期化
	m_gbufferRender.Init();
	m_shadowMapRender.Init();		 m_shadowMapRender.SetSetting(initParam.shadowMapSetting);
	m_ambientOcclusionRender.Init(); m_ambientOcclusionRender.SetEnable(initParam.isEnableSSAO);
	m_defferdRender.Init();
	m_motionBlurRender.Init();		 m_motionBlurRender.SetEnable(initParam.isEnableMotionBlur);
	m_ConvertLinearToSRGB.Init();

	if (initParam.isSplitScreen) {
		m_finalRender[0] = std::make_unique<FinalRender>();
		m_finalRender[1] = std::make_unique<FinalRender>();

		if (initParam.isSplitScreen == enVertical_TwoSplit) {
			m_finalRender[1]->Init({ 0.0f,0.0f }, { 1.0f,0.5f });
			m_finalRender[0]->Init({ 0.0f,0.5f }, { 1.0f,1.0f });
		}
		if (initParam.isSplitScreen == enSide_TwoSplit) {
			m_finalRender[0]->Init({ 0.0f,0.0f }, { 0.5f,1.0f });
			m_finalRender[1]->Init({ 0.5f,0.0f }, { 1.0f,1.0f });
		}

		m_cameraSwitchRender[0] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[1] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[0]->Init(0);
		m_cameraSwitchRender[1]->Init(1);
	}
	else {
		m_finalRender[0] = std::make_unique<FinalRender>();
		m_finalRender[0]->Init();
	}
	FinalRender::SetIsLensDistortion(initParam.isLensDistortion);

	//レンダーをセット	
	m_renderManager.AddRender(-1, &m_shadowMapRender);

	int screencnt = 1, oneloopOffset = 5000;
	if (initParam.isSplitScreen) {
		screencnt = 2;
	}
	for (int i = 0; i < screencnt; i++) {

		int offset = oneloopOffset * i;		

		if (initParam.isSplitScreen) {
			m_renderManager.AddRender(0 + offset, m_cameraSwitchRender[i].get());
		}

		m_renderManager.AddRender(1 + offset, &m_gbufferRender);

		m_renderManager.AddRender(3 + offset, &m_ambientOcclusionRender);

		m_renderManager.AddRender(4 + offset, &m_defferdRender);
		m_renderManager.AddRender(5 + offset, &m_motionBlurRender);

		m_renderManager.AddRender(6 + offset, &m_ConvertLinearToSRGB);

		m_renderManager.AddRender(7 + offset, &m_effekseerRender);

#ifndef DW_MASTER
		m_renderManager.AddRender(999 + offset, &m_physicsDebugDrawRender);
#endif
		
		m_renderManager.AddRender(1000 + offset, m_finalRender[i].get());		
	}
}

//描画先を最終レンダーターゲットにする
void GraphicsEngine::SetFinalRenderTarget() {
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), m_FRT.GetDSV());
}

}