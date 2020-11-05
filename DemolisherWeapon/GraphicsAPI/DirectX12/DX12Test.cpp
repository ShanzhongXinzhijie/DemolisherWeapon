#include "DWstdafx.h"
#include "DX12Test.h"
#include "GraphicsAPI/DirectX12/d3dx12.h"
#include "Graphic/ReyTracing/ReyTracingEngine.h"

#include<comdef.h>

namespace DemolisherWeapon {
	
	struct MeshTest
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;
		Shader m_vs, m_ps;
		CModelMeshParts m_mesh;

		struct ConstantBuffer {
			CMatrix mWorld;
			CMatrix mView;
			CMatrix mProj;
		};
		ConstantBuffer m_cbData;
		ConstantBufferDx12<ConstantBuffer> m_cb;
	};	

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
			HRESULT hr;
			try
			{
				hr = D3D12CreateDevice(dxgiAdapter.Get(), fuatureLevel, IID_PPV_ARGS(&m_d3dDevice));
			}
			catch (const _com_error& ce)
			{
				DW_ERRORBOX(true,ce.ErrorMessage())
			}

			if (SUCCEEDED(hr)) {
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
		swapChainDesc.Width = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W());
		swapChainDesc.Height = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H());
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		//フルスクリーン設定
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDescPtr = nullptr;
		if (!initParam.isWindowMode) {
			fullscreenDescPtr = &fullscreenDesc;//使う		

			auto refleshRate = Util::GetRefreshRate(hWnd);//リフレッシュレートを取得
			fullscreenDesc.RefreshRate.Numerator = refleshRate;
			fullscreenDesc.RefreshRate.Denominator = 1;

			fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			fullscreenDesc.Windowed = false;
		}

		ComPtr<IDXGISwapChain1> swapChainTmp;
		auto hr = dxgiFactory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			hWnd,
			&swapChainDesc,
			fullscreenDescPtr,
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
		{
			D3D12_CLEAR_VALUE dsvClearValue;
			dsvClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			dsvClearValue.DepthStencil.Depth = 1.0f;
			dsvClearValue.DepthStencil.Stencil = 0;

			CD3DX12_RESOURCE_DESC desc(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W()),
				static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H()),
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
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

			m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, dsvHandle);
		}

		//CBV_SRV_UAV用のデスクリプタヒープ作成
		m_srvsDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CBV_SRV_UAV_MAXNUM, false, m_srvsDescriptorHeap);
		//ダミーハンドル作成
		m_srvIndex++;
		m_srvsDammyCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

		//描画用
		m_drawSRVsDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DRW_SRVS_DESC_NUM, true, m_drawSRVsDescriptorHeap);

		//サンプラー用のデスクリプタヒープ作成
		m_samplerDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SAMPLER_MAXNUM, true, m_samplerDescriptorHeap);

		// コマンドアロケータを作成する.
		for (int i = 0; i < FRAME_COUNT; ++i) {
			if (FAILED(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])))) {
				return false;
			}
			wchar_t name[32];
			swprintf_s(name, L"m_commandAllocator[%d]", i);
			m_commandAllocator[i]->SetName(name);
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
		m_commandList->SetName(L"DWCommandList");

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
	}

	void DX12Test::Release(){
		WaitForGpu();
		CloseHandle(m_fenceEvent);
		//Report();
	}

	void DX12Test::Report() {
#ifndef DW_MASTER
		//Release忘れの出力
		ID3D12DebugDevice* debugInterface;
		if (SUCCEEDED(m_d3dDevice->QueryInterface(&debugInterface)))
		{
			HRESULT hr = debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			debugInterface->Release();
		}
#endif
	}

	bool DX12Test::RenderInit() {
		if (m_isInitTest)return true;

		//シェーダのロード
		m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);
		
		//ルートシグネチャの作成
		D3D12_DESCRIPTOR_RANGE descRange[] = {
			CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0),//0番にいっこ?
			CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0)
		};

		CD3DX12_ROOT_PARAMETER rootParameters[1];
		rootParameters[0].InitAsDescriptorTable(_countof(descRange), descRange);

		D3D12_STATIC_SAMPLER_DESC staticSampler[] = { CD3DX12_STATIC_SAMPLER_DESC(0) };

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {
			_countof(rootParameters),
			rootParameters,
			_countof(staticSampler),
			staticSampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		};

		Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
		if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, nullptr))) {
			return false;
		}
		if (FAILED(m_d3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)))) {
			return false;
		}

		//パイプラインステートオブジェクトの作成
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.GetBlob());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.GetBlob());
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = 0xffffffff;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.InputLayout.pInputElementDescs = SVertex::vertexLayout;
		psoDesc.InputLayout.NumElements = sizeof(SVertex::vertexLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc = { 1, 0 };
		//if (isWarp) {
		//	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
		//}
		if (FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)))) {
			return false;
		}

		//プリミティブ
		SVertex vertex[4];
		unsigned long index[4] = { 1,0,3,2 };
		vertex[0] = {
			{0.2f * 2.0f - 1.0f, 0.1f * 2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		};
		vertex[1] = {
			{0.7f * 2.0f - 1.0f, 0.3f * 2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		};
		vertex[2] = {
			{0.3f * 2.0f - 1.0f, 0.7f * 2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		};
		vertex[3] = {
			{0.7f * 2.0f - 1.0f, 0.7f * 2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		};
		m_square.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, 4, index);

		//テクスチャ
		//m_texture2 = CreateTexture(L"utc_all2.dds");		
		//m_texture = CreateTexture(L"utc_all2.dds");
		const TextueData* return_textureData;
		TextureFactory::GetInstance().Load(L"utc_all2.dds", &return_textureData, true);//TODO これのあとテクスチャロード不能?
		m_texture = *return_textureData;

		//モデル
		m_meshTest = new MeshTest;
		tkEngine::CTkmFile tkmFile;
		tkmFile.Load("Assets/modelData/unityChan.tkm");
		m_meshTest->m_mesh.InitFromTkmFile(tkmFile);
		m_meshTest->m_vs.Load("Preset/shader/ModelDX12Test.fx", "VSMainNonSkin", Shader::EnType::VS);
		m_meshTest->m_ps.Load("Preset/shader/ModelDX12Test.fx", "PSMain", Shader::EnType::PS);
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_meshTest->m_vs.GetBlob());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_meshTest->m_ps.GetBlob());
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.SampleMask = 0xffffffff;
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.InputLayout.pInputElementDescs = VertexPositionNormalTangentColorTexture::InputElementsDX12;
			psoDesc.InputLayout.NumElements = sizeof(VertexPositionNormalTangentColorTexture::InputElementsDX12) / sizeof(D3D12_INPUT_ELEMENT_DESC);
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc = { 1, 0 };
			HRESULT hr = m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_meshTest->m_pso));
			if (FAILED(hr)) {
				return false;
			}
		}		
		m_meshTest->m_cb.Init(sizeof(m_meshTest->m_cbData));
		m_meshTest->m_cb.CreateConstantBufferView();

		m_rayTraceTestModel.LoadTkmFile("Assets/modelData/unityChan.tkm");
		m_rayTraceTestModel.CreateMeshParts();
		
		m_rayTraceTestModel.FindMesh([&](const std::unique_ptr<SModelMesh>& m) {m_texture = m->m_materials[0]->GetMaterialData().GetDefaultAlbedoTexture(); });

		//描画ヒープにコピー
		m_d3dDevice->CopyDescriptorsSimple(
			1,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_drawSRVsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, m_drawSRVsDescriptorSize),//dest
				m_texture.CPUdescriptorHandle,//src
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);
		m_d3dDevice->CopyDescriptorsSimple(
			1,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_drawSRVsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_drawSRVsDescriptorSize),//dest
			m_meshTest->m_cb.GetCPUDescriptorHandle(),//src
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

		SetMainCamera(&m_camera);
		m_camera.SetPos({ 0,50,-200 });
		m_camera.SetTarget({0,50,0});
		m_camera.SetFar(10000.0f);

		//レイトレーシング		
		m_rayTraceEngine = new RayTracingEngine;
		m_rayTraceEngine->RegistGeometry(m_rayTraceTestModel);
		m_rayTraceEngine->CommitRegistGeometry(m_commandList.Get());

		//初期化完了
		m_isInitTest = true;
		return true;
	}

	void DX12Test::Render() {
		//RenderInit();

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
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		//レンダーターゲットを塗りつぶし
		CVector4 clearColor = { 1.0f, 0.2f, 0.4f, 1.0f };		
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor.v, 0, nullptr);
		//デプスのクリア
		GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//初期化
		RenderInit();

		//ディスクリプタヒープの設定
		//ID3D12DescriptorHeap* heapList[] = { m_srvsDescriptorHeap.Get() };
		//m_commandList->SetDescriptorHeaps(_countof(heapList), heapList);

		//ビューポート設定
		SetViewport(0.0f, 0.0f, GetGraphicsEngine().Get3DFrameBuffer_W(), GetGraphicsEngine().Get3DFrameBuffer_H());

		//四角形の描画
		//{
		//	//頂点の更新
		//	SVertex vertex[4];
		//	if (cnt < 30) {
		//		vertex[0] = {
		//				{0.3f * 2.0f - 1.0f, 0.3f * 2.0f - 1.0f, 0.0f, 1.0f},
		//				{0.0f, 1.0f}
		//		};
		//	}
		//	else {
		//		vertex[0] = {
		//			{0.2f * 2.0f - 1.0f, 0.1f * 2.0f - 1.0f, 0.0f, 1.0f},
		//			{0.0f, 1.0f}
		//		};
		//	}
		//	vertex[1] = {
		//		{0.7f * 2.0f - 1.0f, 0.3f * 2.0f - 1.0f, 0.0f, 1.0f},
		//		{1.0f, 1.0f}
		//	};
		//	vertex[2] = {
		//		{0.3f * 2.0f - 1.0f, 0.7f * 2.0f - 1.0f, 0.0f, 1.0f},
		//		{0.0f, 0.0f}
		//	};
		//	vertex[3] = {
		//		{0.7f * 2.0f - 1.0f, 0.7f * 2.0f - 1.0f, 0.0f, 1.0f},
		//		{1.0f, 0.0f}
		//	};
		//	m_square.UpdateVertex(vertex);

		//	m_commandList->SetPipelineState(m_pso.Get());
		//	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		//	//m_commandList->SetDescriptorHeaps(1, m_srvsDescriptorHeap.GetAddressOf());
		//	m_commandList->SetGraphicsRootDescriptorTable(0, m_texture.GPUdescriptorHandle);
		//	//commandList->SetGraphicsRoot32BitConstants(0, 16, &matViewProjection, 0);
		//	//m_square.DrawIndexed();
		//}

		//
		//static float rot = 0;
		//rot += CMath::RadToDeg(0.001f);
		//m_meshTest->m_cbData.mWorld.Identity();// .MakeRotationX(rot);
		//m_meshTest->m_cbData.mView = GetMainCamera()->GetViewMatrix();
		//m_meshTest->m_cbData.mProj = GetMainCamera()->GetProjMatrix();
		//m_meshTest->m_cb.Update(&m_meshTest->m_cbData);

		////モデルの描画
		//m_commandList->SetPipelineState(m_meshTest->m_pso.Get());
		//m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		//ID3D12DescriptorHeap* heapList[] = { m_drawSRVsDescriptorHeap.Get() };
		//m_commandList->SetDescriptorHeaps(_countof(heapList), heapList);
		//m_commandList->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_drawSRVsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_drawSRVsDescriptorSize));
		////m_meshTest->m_mesh.Draw(1);
		//m_rayTraceTestModel.Draw(1);

		//レイトレ
		m_rayTraceEngine->Dispatch(m_commandList.Get());

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
		m_swapChain->Present(GetGraphicsEngine().GetUseVSync() ? 1 : 0, 0);

		//フェンスのインクリメント
		m_fenceValue[m_currentBackBufferIndex]++;
		//フェンスの値変更
		if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
			return;
		}
	}

	void DX12Test::SetBackBufferToRenderTarget() {		
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
	}

	void DX12Test::ExecuteCommand() {
		//リソースバリアを設定
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		//コマンドリストを閉じる
		if (FAILED(m_commandList->Close())) {
			return;
		}

		//コマンドリスト実行
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//フェンスのインクリメント
		m_fenceValue[m_currentBackBufferIndex]++;

		//フェンスの値変更
		if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
			return;
		}
	}

	void DX12Test::SwapBackBuffer() {
		//バックバッファとフロントバッファを入れ替える。
		m_swapChain->Present(GetGraphicsEngine().GetUseVSync() ? 1 : 0, 0);
	}

	void DX12Test::SetViewport(float topLeftX, float topLeftY, float width, float height) {
		m_viewport.Width = width;
		m_viewport.Height = height;
		m_viewport.TopLeftX = topLeftX;
		m_viewport.TopLeftY = topLeftY;
		m_viewport.MinDepth = D3D12_MIN_DEPTH;
		m_viewport.MaxDepth = D3D12_MAX_DEPTH;
		GetCommandList()->RSSetViewports(1, &m_viewport);
#ifdef DW_DX12_TEMPORARY
		GetGraphicsEngine().GetSpriteBatch()->SetViewport(m_viewport);
		GetGraphicsEngine().GetSpriteBatchPMA()->SetViewport(m_viewport);
#endif
		m_scissorRect.right = (LONG)(topLeftX + width);
		m_scissorRect.bottom = (LONG)(topLeftY + height);
		m_scissorRect.left = (LONG)topLeftX;
		m_scissorRect.top = (LONG)topLeftY;
		GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
	}
}