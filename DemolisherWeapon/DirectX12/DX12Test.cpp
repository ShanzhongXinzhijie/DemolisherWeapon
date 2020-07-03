#include "DWstdafx.h"
#include "DX12Test.h"
#include "DirectX12/d3dx12.h"

namespace DemolisherWeapon {

	UINT DX12Test::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool isShaderVisible, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap) {
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = numDescriptors;
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(m_d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)))) {
			return -1;
		}
		//ディスクリプタのサイズを返す
		return m_d3dDevice->GetDescriptorHandleIncrementSize(type);
	}

	bool DX12Test::Init(HWND hWnd, const InitEngineParameter& initParam) {
		using namespace Microsoft::WRL;

		UINT dxgiFactoryFlags = 0;
#ifndef DW_MASTER
		{
			//デバッグコントローラーがあれば、デバッグレイヤーがあるDXGIを作成する。
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif
		//DXGIFactory作成
		ComPtr<IDXGIFactory4> dxgiFactory;
		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)))) {
			return false;
		}

		// 機能レベル11を満たすハードウェアアダプタを検索し、そのデバイスインターフェイスを取得する.
		ComPtr<IDXGIAdapter1> dxgiAdapter, dxgiAdapterTmp; // デバイス情報を取得するためのインターフェイス
		int adapterIndex = 0; // 列挙するデバイスのインデックス
		SIZE_T maxDedicatedVideoMemory = 0;//トップのビデオメモリ
		//ループで探す
		while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapterTmp) != DXGI_ERROR_NOT_FOUND) {
			//情報取得
			DXGI_ADAPTER_DESC1 desc;
			dxgiAdapterTmp->GetDesc1(&desc);

			//TODO アダプタ、オプションで選択できるように

			//HardwareAdapterを使用する & ビデオメモリがより多いもの
			if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) && desc.DedicatedVideoMemory >= maxDedicatedVideoMemory) {
				dxgiAdapter = dxgiAdapterTmp;
				maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
			}
			++adapterIndex;
		}

		// 機能レベル11を満たすハードウェアが見つからない場合、WARPデバイスの作成を試みる.
		if (!dxgiAdapter) {
			if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)))) {
				return false;//もうだめだぁー(BNBN)
			}
		}

		//使用する機能レベル
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};		

		//D3D12デバイスの作成
		for (auto fuatureLevel : featureLevels) {
			if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), fuatureLevel, IID_PPV_ARGS(&m_d3dDevice)))) {
				break;
			}
		}

		{
			// コマンドキューを作成
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			auto hr = m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
			if (FAILED(hr)) {
				return false;
			}
		}

		// スワップチェーンを作成
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FRAME_COUNT;
		swapChainDesc.Width = initParam.frameBufferWidth;
		swapChainDesc.Height = initParam.frameBufferHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChainTmp;
		auto hr = dxgiFactory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChainTmp
		);
		if (FAILED(hr)) {
			return false;
		}

		//IDXGISwapChain3のインターフェースを取得。
		swapChainTmp.As(&m_swapChain);
		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		//RTV用のデスクリプタヒープ作成
		m_rtvDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_COUNT, false, m_rtvDescriptorHeap);

		//RTV作成
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (int i = 0; i < FRAME_COUNT; ++i) {
			auto hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
			if (FAILED(hr)) {
				return false;
			}
			m_d3dDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += m_rtvDescriptorSize;
		}

		//DSV用のデスクリプタヒープ作成
		m_dsvDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false, m_dsvDescriptorHeap);

		//DSV作成
		/*{
			D3D12_CLEAR_VALUE dsvClearValue;
			dsvClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			dsvClearValue.DepthStencil.Depth = 1.0f;
			dsvClearValue.DepthStencil.Stencil = 0;

			CD3DX12_RESOURCE_DESC desc(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT>(initParam.frameBufferWidth),
				static_cast<UINT>(initParam.frameBufferHeight),
				1,
				1,
				DXGI_FORMAT_D32_FLOAT,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

			auto hr = m_d3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&dsvClearValue,
				IID_PPV_ARGS(&m_depthStencilBuffer)
			);
			if (FAILED(hr)) {
				//深度ステンシルバッファの作成に失敗。
				return false;
			}
			//ディスクリプタを作成
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

			m_d3dDevice->CreateDepthStencilView(
				m_depthStencilBuffer.Get(), nullptr, dsvHandle
			);
		}*/

		// コマンドアロケータを作成する.
		for (int i = 0; i < FRAME_COUNT; ++i) {
			if (FAILED(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])))) {
				return false;
			}
		}

		// コマンドリストを作成する.
		//スレッド数分創るとか
		if (FAILED(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_currentBackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)))) {
			return false;
		}
		//コマンドリストは開かれている状態で作成されるので、いったん閉じる。
		if (FAILED(m_commandList->Close())) {
			return false;
		}

		// フェンスとフェンスイベントを作成する.
		if (FAILED(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) {
			return false;
		}
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent) {
			return false;
		}
		for (int i = 0; i < FRAME_COUNT; ++i) {
			m_fenceValue[i] = 0;
		}

		return true;

		/*
			/// <summary>
			/// ///////////
			/// </summary>
			///
			//メインレンダリングターゲットを作成。
			float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
			if (m_mainRenderTarget.Create(
				initParam.frameBufferWidth,
				initParam.frameBufferHeight,
				1,
				1,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_D32_FLOAT,
				clearColor) == false) {
				TK_ASSERT(false, "メインレンダリングターゲットの作成に失敗しました。");
				return false;
			}

			//CBR_SVRのディスクリプタのサイズを取得。
			m_cbrSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			//パイプラインステートを初期化。
			CPipelineStatesDx12::Init();

			//ポストエフェクトの初期化。
			m_postEffect.Init(initParam.graphicsConfing);

			m_copyFullScreenSprite.Init(
				&m_mainRenderTarget.GetRenderTargetTexture(),
				static_cast<float>(initParam.frameBufferWidth),
				static_cast<float>(initParam.frameBufferHeight));
			//ビューポートを初期化。
			m_viewport.TopLeftX = 0;
			m_viewport.TopLeftY = 0;
			m_viewport.Width = static_cast<float>(initParam.frameBufferWidth);
			m_viewport.Height = static_cast<float>(initParam.frameBufferHeight);
			m_viewport.MinDepth = D3D12_MIN_DEPTH;
			m_viewport.MaxDepth = D3D12_MAX_DEPTH;

			//シザリング矩形を初期化。
			m_scissorRect.left = 0;
			m_scissorRect.top = 0;
			m_scissorRect.right = initParam.frameBufferWidth;
			m_scissorRect.bottom = initParam.frameBufferHeight;

			//レンダリングコンテキストの作成。
			auto giFactry = g_engine->GetGraphicsInstanceFactory();
			m_renderContext = giFactry->CreateRenderContext();
			auto& rcDx12 = m_renderContext->As<CRenderContextDx12>();
			rcDx12.SetCommandList(m_commandList);
			*/
	}

	void DX12Test::Release(){
		WaitForGpu();
		CloseHandle(m_fenceEvent);
	}

	void DX12Test::Render() {
		//前フレームの描画完了を待つ
		if (!WaitForPreviousFrame()) {
			return;
		}

		//これから使うコマンドリスト・アロケータをリセットして使用可能に
		if (FAILED(m_commandAllocator[m_currentBackBufferIndex]->Reset())) {
			return;
		}
		if (FAILED(m_commandList->Reset(m_commandAllocator[m_currentBackBufferIndex].Get(), nullptr))) {
			return;
		}

		//リソースバリアを設定
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		//レンダーターゲットを設定
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		//レンダーターゲットを塗りつぶし
		CVector4 clearColor = { 1.0f, 0.2f, 0.4f, 1.0f };
		static int cnt = 0;
		cnt++;
		if (cnt < 30) {
			clearColor = { 1.0f, 0.2f, 0.4f, 1.0f };
		}else
		if (cnt < 60) {
			clearColor = { 0.4f, 0.2f, 1.0f, 1.0f };
		}
		else {
			cnt = 0;
		}
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor.v, 0, nullptr);

		//リソースバリアを設定
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		//コマンドリストを閉じる
		if (FAILED(m_commandList->Close())) {
			return;
		}

		//コマンドリスト実行
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//スワップチェイン
		if (FAILED(m_swapChain->Present(1, 0))) {
			return;
		}

		//フェンスのインクリメント
		/*fenceValue[currentFrameIndex] = masterFenceValue;
		if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
			return;
		}
		++masterFenceValue;*/

		if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
			return;
		}
	}
}