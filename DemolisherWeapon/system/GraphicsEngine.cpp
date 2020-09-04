#include "DWstdafx.h"
#include "GraphicsEngine.h"

namespace DemolisherWeapon {

namespace {
	constexpr int oneloopOffset = 5000;
}

GraphicsEngine::GraphicsEngine(): m_primitiveRender2D(&m_primitiveRender)
{
	
}
GraphicsEngine::~GraphicsEngine()
{
	Release();
}

void GraphicsEngine::ClearBackBuffer()
{
	m_graphicsAPI->ClearBackBuffer();
}
void GraphicsEngine::SetBackBufferToRenderTarget()
{
	m_graphicsAPI->SetBackBufferToRenderTarget();
}
void GraphicsEngine::SwapBackBuffer()
{
	m_graphicsAPI->SwapBackBuffer();
}
void GraphicsEngine::ExecuteCommand() {
	m_graphicsAPI->ExecuteCommand();
}
void GraphicsEngine::RunRenderManager() {
	m_renderManager.Render();
}

void GraphicsEngine::Release() {
	m_FRT.Release();
	
	m_useAPI = enNum;
	m_graphicsAPI.reset();
	m_dx11 = nullptr;
	m_dx12 = nullptr;

	m_xtk12_resourceDescriptors.Release();
}

bool GraphicsEngine::Init(HWND hWnd, const InitEngineParameter& initParam, GameObjectManager* gom, CFpsCounter* fc) {
	//フレームバッファサイズ
	FRAME_BUFFER_W = (float)initParam.frameBufferWidth;
	FRAME_BUFFER_H = (float)initParam.frameBufferHeight;

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

	//垂直同期設定
	m_useVSync = initParam.useVSync;

	//レンダークラスに参照渡す
	m_directxtkRender.Init(gom, fc);

	//グラフィックスAPIごとの初期化
#ifdef DW_DX12
	return InnerInitDX12(hWnd, initParam);
#else
	return InnerInitDX11(hWnd, initParam);
#endif	
}

#ifdef DW_DX12
bool GraphicsEngine::InnerInitDX12(HWND hWnd, const InitEngineParameter& initParam) {
	//DirectX12初期化
	m_useAPI = enDirectX12;
	m_graphicsAPI = std::make_unique<DX12Test>();
	auto sucsses = m_graphicsAPI->Init(hWnd, initParam);
	if (!sucsses) {
		return false;
	}
	m_dx12 = dynamic_cast<DX12Test*>(m_graphicsAPI.get());

	//sprite初期化
	{
		m_commonStates = std::make_unique<DirectX::CommonStates>(m_dx12->GetD3D12Device());

		DirectX::ResourceUploadBatch resourceUpload(m_dx12->GetD3D12Device());

		//ディスクリプタヒープ作る
		m_xtk12_resourceDescriptors.Init(m_dx12->GetD3D12Device(), initParam.xtk12DescriptorsMaxnum);

		// コマンドキューを作成
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		auto hr = m_dx12->GetD3D12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_xtk12_commandQueue));
		if (FAILED(hr)) {
			return false;
		}
		//なんか作る
		m_xtk12_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_dx12->GetD3D12Device());

		//コマンドリスト作る
		resourceUpload.Begin();
			
		//フォント作成
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		m_xtk12_resourceDescriptors.CreateDescriptorNumber(cpuHandle,gpuHandle);
		m_spriteFont = std::make_unique<DirectX::SpriteFont>(
			m_dx12->GetD3D12Device(), 
			resourceUpload,
			L"Preset/Font/myfile.spritefont",
			cpuHandle,
			gpuHandle
		);

		//スプライトバッチ作成		
		DirectX::RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);//レンダーターゲットの情報がいる(バックバッファのものを使用)
		{//alphaBlend
			//パイプラインステート(ブレンドステートとかシェーダとか設定する)
			DirectX::SpriteBatchPipelineStateDescription pd(rtState);
			//状態の組み合わせごとにSpriteBatch(Pipeline State Object<PSO>)が必要
			m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_dx12->GetD3D12Device(), resourceUpload, pd);			
		}
		{//PMABlend
			//パイプラインステート(ブレンドステートとかシェーダとか設定する)
			DirectX::SpriteBatchPipelineStateDescription pd(rtState, &DirectX::CommonStates::NonPremultiplied);
			//状態の組み合わせごとにSpriteBatch(Pipeline State Object<PSO>)が必要
			m_spriteBatchPMA = std::make_unique<DirectX::SpriteBatch>(m_dx12->GetD3D12Device(), resourceUpload, pd);			
		}

		//コマンドリストをキューへ送る
		auto uploadResourcesFinished = resourceUpload.End(m_xtk12_commandQueue.Get());

		//処理終わり待つ
		uploadResourcesFinished.wait();
	}

	//レンダーの登録
	m_dx12Render.Init(m_dx12);
	m_renderManager.AddRender(-2, &m_dx12Render);
	
	
	//初期化レンダー
	//m_renderManager.AddRender(-3, &m_initRender);

	//int screencnt = m_isSplitScreen ? 2 : 1;
	//int offset = oneloopOffset * (screencnt + 1);
	////2dinit
	//m_renderManager.AddRender(offset + 1, &m_initRender2D);//ビューポート設定コマンドリストへ
	////primrender2D
	////m_renderManager.AddRender(offset + 2, &m_primitiveRender2D);

	////DirectXTKRender
	//m_renderManager.AddRender(offset + 3, &m_directxtkRender);

	////finishrender
	//m_renderManager.AddRender(offset + 4, &m_SUSRTFinishRender);	
	//

	return true;
}
#endif

bool GraphicsEngine::InnerInitDX11(HWND hWnd, const InitEngineParameter& initParam) {	
	//DirectX11初期化
	m_useAPI = enDirectX11;
	m_graphicsAPI = std::make_unique<DX11Test>();
	auto sucsses = m_graphicsAPI->Init(hWnd, initParam);
	if (!sucsses) {
		return false;
	}
	m_dx11 = dynamic_cast<DX11Test*>(m_graphicsAPI.get());

	//シェーダーのパス設定(デバッグ用)
	ShaderResources::GetInstance().SetIsReplaceForEngineFilePath(initParam.isShaderPathReplaceForEngineFilePath);
	//シェーダーの再コンパイル設定
	ShaderResources::GetInstance().SetIsRecompile(initParam.isShaderRecompile);

	//ビューポートを初期化。
	SetViewport(0.0f, 0.0f, FRAME_BUFFER_W, FRAME_BUFFER_H);

#ifdef DW_DX11
	//ブレンドステート初期化
	m_commonStates = std::make_unique<DirectX::CommonStates>(GetD3DDevice());
	GetD3DDeviceContext()->OMSetBlendState(m_commonStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);

	//Sprite初期化
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(GetD3DDevice(), L"Preset/Font/myfile.spritefont");
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(GetD3DDeviceContext());
	m_spriteBatchPMA = std::make_unique<DirectX::SpriteBatch>(GetD3DDeviceContext());

	//ロード画面描画
	{
		//背景色
		//float ClearColor[4] = { 0.0f,0.3f,0.95f,1.0f };
		//m_pd3dDeviceContext->ClearRenderTargetView(m_backBuffer, ClearColor);
		//m_pd3dDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//描画先をバックバッファにする
		SetBackBufferToRenderTarget();
		
		//2D用の設定にする
		SetViewport(0.0f, 0.0f, GetFrameBuffer_W(), GetFrameBuffer_H());

		//テキスト
		GetSpriteBatch()->Begin();// DirectX::SpriteSortMode::SpriteSortMode_Deferred, GetGraphicsEngine().GetCommonStates().NonPremultiplied());
		GetSpriteFont()->DrawString(
			GetEngine().GetGraphicsEngine().GetSpriteBatch(),
			L"ピョピグプフヒネプを実行中\n"
			L"初回起動時は時間がかかります...(すごく)",
			{ 0.5f * GetFrameBuffer_W(), 0.5f * GetFrameBuffer_H() },
			{ 1.0f,1.0f,1.0f,1.0f }, 0.0f, DirectX::XMFLOAT2(0.5f, 0.5f), 0.5f
		);
		GetSpriteBatch()->End();

		//バックバッファを表へ
		SwapBackBuffer();
	}
#endif

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

		//HUD描画レンダー
		for (int i = 0; i < 2; i++) {
			CVector2 areaMin, areaMax;
			m_finalRender[i]->GetDrawArea(areaMin, areaMax);
			m_HUDRender[i] = std::make_unique<HUDRender>();
			m_HUDRender[i]->Init(i, areaMin, areaMax, { (float)initParam.HUDWidth, (float)initParam.HUDHeight });
		}

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

		//HUD描画レンダー
		for (int i = 0; i < 1; i++) {
			CVector2 areaMin, areaMax;
			m_finalRender[i]->GetDrawArea(areaMin, areaMax);
			m_HUDRender[i] = std::make_unique<HUDRender>();
			m_HUDRender[i]->Init(i, areaMin, areaMax, { (float)initParam.HUDWidth, (float)initParam.HUDHeight });
		}
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

		//HUD描画
		m_renderManager.AddRender(1001 + offset, m_HUDRender[i].get());
	}	
	int offset = oneloopOffset * (screencnt + 1);
	//2dinit
	m_renderManager.AddRender(offset + 1, &m_initRender2D);
	//primrender2D
	m_renderManager.AddRender(offset + 2, &m_primitiveRender2D);
	//DirectXTKRender
	m_renderManager.AddRender(offset + 3, &m_directxtkRender);
	//finishrender
	m_renderManager.AddRender(offset + 4, &m_SUSRTFinishRender);

	//GPUイベント用
#ifndef DW_MASTER
	m_dx11->GetD3DDeviceContext()->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_pUserAnotation);
#endif

	return true;
}

void GraphicsEngine::ChangeFrameBufferSize(int frameBufferWidth, int frameBufferHeight, int frameBufferWidth3D, int frameBufferHeight3D, int HUDWidth, int HUDHeight, EnSplitScreenMode screenMode, float* splitScreenSize) {
	//サイズ変更
	FRAME_BUFFER_W = (float)frameBufferWidth;
	FRAME_BUFFER_H = (float)frameBufferHeight;
	
	//グラフィックスApiごとの処理
	m_graphicsAPI->ChangeFrameBufferSize();

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
	//HUDリサイズ
	for (auto& hud : m_HUDRender) {
		if (hud) {
			hud->Resize({ (float)HUDWidth, (float)HUDHeight });
		}
	}

	//画面分割変更
	if (isChangeSplitScreen) {		
		//再確保
		m_finalRender[0].reset();
		m_finalRender[1].reset();
		m_preRenderRender[0].reset();
		m_preRenderRender[1].reset();
		m_cameraSwitchRender[0].reset();
		m_cameraSwitchRender[1].reset();
		m_HUDRender[0].reset();
		m_HUDRender[1].reset();

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

		//HUDレンダー
		for (int i = 0; i < (m_isSplitScreen ? 2 : 1); i++) {
			CVector2 areaMin, areaMax;
			m_finalRender[i]->GetDrawArea(areaMin, areaMax);
			m_HUDRender[i] = std::make_unique<HUDRender>();
			m_HUDRender[i]->Init(i, areaMin, areaMax, { (float)HUDWidth, (float)HUDHeight });
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
				m_renderManager.DeleteRender(1001 + offset);

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
					//HUD描画
					m_renderManager.AddRender(1001 + offset, m_HUDRender[i].get());
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