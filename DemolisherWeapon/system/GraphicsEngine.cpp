#include "DWstdafx.h"
#include "GraphicsEngine.h"

namespace DemolisherWeapon {

	namespace {
		static constexpr int oneloopOffset = 5000;
	}

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
	sd.BufferDesc.RefreshRate.Numerator = initParam.refleshRate;//モニタのリフレッシュレート。(バックバッファとフロントバッファを入れ替えるタイミングとなる。)
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

	//ブレンドステート初期化
	m_commonStates = std::make_unique<DirectX::CommonStates>(m_pd3dDevice);
	m_pd3dDeviceContext->OMSetBlendState(m_commonStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);

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
	m_isSplitScreen = initParam.isSplitScreen;
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
	m_fullscreen.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, m_vertex, 4, m_index);

	//ライトマネージャー
	m_lightManager.Init();

	//最終レンダーターゲット
	m_FRT.Init();

	//レンダー初期化
	m_gbufferRender.Init();
	m_shadowMapRender.SetSetting(initParam.shadowMapSetting);
	m_postDrawModelRender.Init();
	m_ambientOcclusionRender.Init(initParam.SSAOBufferScale);	 m_ambientOcclusionRender.SetEnable(initParam.isEnableSSAO);
	m_defferdRender.Init();
	m_bloomRender.Init(initParam.bloomBufferScale);				 m_bloomRender.SetEnable(initParam.isEnableBloom);
	m_DOFRender.Init(initParam.DOFBufferScale);					 m_DOFRender.SetEnable(initParam.isEnableDOF);
	m_motionBlurRender.Init();									 m_motionBlurRender.SetEnable(initParam.isEnableMotionBlur);
	m_ConvertLinearToSRGB.Init();
	m_primitiveRender.Init();

	if (initParam.isSplitScreen) {
		//画面分割初期化

		//最終レンダー
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

		//描画前処理レンダー
		m_preRenderRender[0] = std::make_unique<PreRenderRender>();
		m_preRenderRender[1] = std::make_unique<PreRenderRender>();
		m_preRenderRender[0]->Init(0);
		m_preRenderRender[1]->Init(1);

		//カメラ切り替えレンダー
		m_cameraSwitchRender[0] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[1] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[0]->Init(0);
		m_cameraSwitchRender[1]->Init(1);
	}
	else {
		//単画面初期化

		//最終レンダー
		m_finalRender[0] = std::make_unique<FinalRender>();
		m_finalRender[0]->Init();

		//描画前処理レンダー
		m_preRenderRender[0] = std::make_unique<PreRenderRender>();
		m_preRenderRender[0]->Init(0);
	}
	FinalRender::SetIsLensDistortion(initParam.isLensDistortion);
	FinalRender::SetIsAntiAliasing(initParam.isAntiAliasing);

	//レンダーをセット	

	//初期化レンダー
	m_renderManager.AddRender(-3, &m_initRender);
	//シャドウマップ描画
	m_renderManager.AddRender(-2, &m_shadowMapRender);

	//画面分割数分実行
	int screencnt = m_isSplitScreen ? 2 : 1;
	for (int i = 0; i < screencnt; i++) {

		int offset = oneloopOffset * i;		

		if (initParam.isSplitScreen) {
			//画面分割ならカメラ切り替え
			m_renderManager.AddRender(-1 + offset, m_cameraSwitchRender[i].get());			
		}

		//描画前処理
		m_renderManager.AddRender(0 + offset, m_preRenderRender[i].get());
		
		//Gバッファ描画
		m_renderManager.AddRender(1 + offset, &m_gbufferRender);

		//AOマップ作成
		m_renderManager.AddRender(3 + offset, &m_ambientOcclusionRender);

		//ディファードレンダリング
		m_renderManager.AddRender(4 + offset, &m_defferdRender);

		//ポストプロセス
		m_renderManager.AddRender(5 + offset, &m_DOFRender);
		m_renderManager.AddRender(6 + offset, &m_motionBlurRender);
		m_renderManager.AddRender(7 + offset, &m_bloomRender);

		//ポストドローモデル
		m_renderManager.AddRender(8 + offset, &m_postDrawModelRender);

		//SRGBに変換
		m_renderManager.AddRender(9 + offset, &m_ConvertLinearToSRGB);

		//Effekseerの描画
		m_renderManager.AddRender(10 + offset, &m_effekseerRender);
		
		m_freeRenderPriority = 11;//ここから↓まで未使用

		//プリミティブ描画
		m_renderManager.AddRender(998 + offset, &m_primitiveRender);

#ifndef DW_MASTER
		//BUlletPhysicsのデバッグ描画
		m_renderManager.AddRender(999 + offset, &m_physicsDebugDrawRender);
#endif
		//最終描画
		m_renderManager.AddRender(1000 + offset, m_finalRender[i].get());		
	}

	//GPUイベント用
#ifndef DW_MASTER
	m_pd3dDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_pUserAnotation);
#endif
}

void GraphicsEngine::ChangeFrameBufferSize(int frameBufferWidth, int frameBufferHeight, int frameBufferWidth3D, int frameBufferHeight3D, EnSplitScreenMode screenMode, float* splitScreenSize) {
	//サイズ変更
	FRAME_BUFFER_W = (float)frameBufferWidth;
	FRAME_BUFFER_H = (float)frameBufferHeight;
	
	//※スワップチェインを参照しているオブジェクトを削除してないとm_pSwapChain->ResizeBuffersができない
	//バックバッファ死亡!!!死!!
	if (m_backBuffer) { m_backBuffer->Release(); m_backBuffer = NULL; }

	//スワップチェインサイズ変更
	DXGI_SWAP_CHAIN_DESC sd;
	m_pSwapChain->GetDesc(&sd);
	HRESULT hr = m_pSwapChain->ResizeBuffers(0, (UINT)FRAME_BUFFER_W, (UINT)FRAME_BUFFER_H, sd.BufferDesc.Format, sd.Flags);

	//バックバッファ再作成
	ID3D11Texture2D* pBackBuffer = NULL;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_backBuffer);
	pBackBuffer->Release();

	//深度ステンシルの設定コピー
	D3D11_TEXTURE2D_DESC texDesc;
	m_depthStencil->GetDesc(&texDesc);
	texDesc.Width = (UINT)FRAME_BUFFER_W;
	texDesc.Height = (UINT)FRAME_BUFFER_H;	
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

	//ビューポートを初期化。
	SetViewport(0.0f, 0.0f, FRAME_BUFFER_W, FRAME_BUFFER_H);

	//画面分割に変更あるか?
	bool isChangeSplitScreen = false;
	if (m_isSplitScreen == enNoSplit && screenMode != enNoSplit || m_isSplitScreen != enNoSplit && screenMode == enNoSplit) {
		isChangeSplitScreen = true;
	}
	m_isSplitScreen = screenMode;

	//画面分割用の比率に
	FRAME_BUFFER_3D_W = (float)frameBufferWidth3D;
	FRAME_BUFFER_3D_H = (float)frameBufferHeight3D;
	if (m_isSplitScreen == enVertical_TwoSplit) {
		FRAME_BUFFER_3D_H *= 0.5f;
	}
	if (m_isSplitScreen == enSide_TwoSplit) {
		FRAME_BUFFER_3D_W *= 0.5f;
	}
	
	//最終レンダーターゲット再初期化
	m_FRT.Init();

	//レンダーリサイズ
	m_gbufferRender.Resize();
	m_postDrawModelRender.Resize();
	m_ambientOcclusionRender.Resize();
	m_defferdRender.Resize();
	m_bloomRender.Resize();	
	m_DOFRender.Resize();	
	m_motionBlurRender.Resize();	
	m_ConvertLinearToSRGB.Resize();
	m_primitiveRender.Resize();		

	//画面分割変更
	if (isChangeSplitScreen) {
		
		//再確保
		m_finalRender[0].reset();
		m_finalRender[1].reset();
		m_preRenderRender[0].reset();
		m_preRenderRender[1].reset();
		m_cameraSwitchRender[0].reset();
		m_cameraSwitchRender[1].reset();

		if (m_isSplitScreen) {
			//画面分割時初期化

			//最終レンダーの確保
			m_finalRender[0] = std::make_unique<FinalRender>();
			m_finalRender[1] = std::make_unique<FinalRender>();
			//スクリーンサイズ設定
			if (splitScreenSize) {
				m_finalRender[0]->Init({ splitScreenSize[0],splitScreenSize[1] }, { splitScreenSize[2],splitScreenSize[3] });
				m_finalRender[1]->Init({ splitScreenSize[4],splitScreenSize[5] }, { splitScreenSize[6],splitScreenSize[7] });
			}
			else {
				if (m_isSplitScreen == enVertical_TwoSplit) {
					m_finalRender[1]->Init({ 0.0f,0.0f }, { 1.0f,0.5f });
					m_finalRender[0]->Init({ 0.0f,0.5f }, { 1.0f,1.0f });
				}
				if (m_isSplitScreen == enSide_TwoSplit) {
					m_finalRender[0]->Init({ 0.0f,0.0f }, { 0.5f,1.0f });
					m_finalRender[1]->Init({ 0.5f,0.0f }, { 1.0f,1.0f });
				}
			}

			//描画前処理レンダー
			m_preRenderRender[0] = std::make_unique<PreRenderRender>();
			m_preRenderRender[1] = std::make_unique<PreRenderRender>();
			m_preRenderRender[0]->Init(0);
			m_preRenderRender[1]->Init(1);

			//カメラ切り替えレンダーの初期化
			m_cameraSwitchRender[0] = std::make_unique<CameraSwitchRender>();
			m_cameraSwitchRender[1] = std::make_unique<CameraSwitchRender>();
			m_cameraSwitchRender[0]->Init(0);
			m_cameraSwitchRender[1]->Init(1);
		}
		else {
			//単画面時初期化

			//最終レンダーの初期化
			m_finalRender[0] = std::make_unique<FinalRender>();
			m_finalRender[0]->Init();

			//描画前処理レンダー
			m_preRenderRender[0] = std::make_unique<PreRenderRender>();
			m_preRenderRender[0]->Init(0);
		}

		//画面分割数分実行
		int screencnt = 2;// m_isSplitScreen ? 2 : 1;
		for (int i = 0; i < screencnt; i++) {

			int offset = oneloopOffset * i;

			//再確保組の再登録
			{
				//レンダーリストから削除
				m_renderManager.DeleteRender(-1 + offset);
				m_renderManager.DeleteRender(0 + offset);
				m_renderManager.DeleteRender(1000 + offset);

				//登録
				if (m_isSplitScreen) {
					//カメラ切り替え
					m_renderManager.AddRender(-1 + offset, m_cameraSwitchRender[i].get());
				}
				if (m_isSplitScreen || i == 0) {
					//描画前処理
					m_renderManager.AddRender(0 + offset, m_preRenderRender[i].get());
					//最終描画
					m_renderManager.AddRender(1000 + offset, m_finalRender[i].get());					
				}				
			}

			if (i == 0) { 
				continue;//この先は一週目は実行しない
			}

			if (m_isSplitScreen) {
				//追加

				//Gバッファ描画
				m_renderManager.AddRender(1 + offset, &m_gbufferRender);

				//AOマップ作成
				m_renderManager.AddRender(3 + offset, &m_ambientOcclusionRender);

				//ディファードレンダリング
				m_renderManager.AddRender(4 + offset, &m_defferdRender);

				//ポストプロセス
				m_renderManager.AddRender(5 + offset, &m_DOFRender);
				m_renderManager.AddRender(6 + offset, &m_motionBlurRender);
				m_renderManager.AddRender(7 + offset, &m_bloomRender);

				//ポストドローモデル
				m_renderManager.AddRender(8 + offset, &m_postDrawModelRender);

				//SRGBに変換
				m_renderManager.AddRender(9 + offset, &m_ConvertLinearToSRGB);

				//Effekseerの描画
				m_renderManager.AddRender(10 + offset, &m_effekseerRender);

				//m_freeRenderPriority = 11;//ここから↓まで未使用

				//プリミティブ描画
				m_renderManager.AddRender(998 + offset, &m_primitiveRender);

#ifndef DW_MASTER
				//BUlletPhysicsのデバッグ描画
				m_renderManager.AddRender(999 + offset, &m_physicsDebugDrawRender);
#endif
			}
			else {
				//削除

				//Gバッファ描画
				m_renderManager.DeleteRender(1 + offset);

				//AOマップ作成
				m_renderManager.DeleteRender(3 + offset);

				//ディファードレンダリング
				m_renderManager.DeleteRender(4 + offset);

				//ポストプロセス
				m_renderManager.DeleteRender(5 + offset);
				m_renderManager.DeleteRender(6 + offset);
				m_renderManager.DeleteRender(7 + offset);

				//ポストドローモデル
				m_renderManager.DeleteRender(8 + offset);

				//SRGBに変換
				m_renderManager.DeleteRender(9 + offset);

				//Effekseerの描画
				m_renderManager.DeleteRender(10 + offset);

				//m_freeRenderPriority = 11;//ここから↓まで未使用

				//プリミティブ描画
				m_renderManager.DeleteRender(998 + offset);

#ifndef DW_MASTER
				//BUlletPhysicsのデバッグ描画
				m_renderManager.DeleteRender(999 + offset);
#endif
			}
		}
	}
}

//描画先を最終レンダーターゲットにする
void GraphicsEngine::SetFinalRenderTarget() {
	GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), m_FRT.GetDSV());
}
//デプス未使用版
void GraphicsEngine::SetFinalRenderTarget_NonDepth() {
	GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), nullptr);
}

}