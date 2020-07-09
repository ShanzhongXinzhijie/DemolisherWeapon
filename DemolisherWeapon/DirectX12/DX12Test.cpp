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
		//�f�B�X�N���v�^�̃T�C�Y��Ԃ�
		return m_d3dDevice->GetDescriptorHandleIncrementSize(type);
	}

	bool DX12Test::Init(HWND hWnd, const InitEngineParameter& initParam) {
		using namespace Microsoft::WRL;

		UINT dxgiFactoryFlags = 0;
#ifndef DW_MASTER
		{
			//�f�o�b�O�R���g���[���[������΁A�f�o�b�O���C���[������DXGI���쐬����B
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif
		//DXGIFactory�쐬
		ComPtr<IDXGIFactory4> dxgiFactory;
		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)))) {
			return false;
		}

		// �@�\���x��11�𖞂����n�[�h�E�F�A�A�_�v�^���������A���̃f�o�C�X�C���^�[�t�F�C�X���擾����.
		ComPtr<IDXGIAdapter1> dxgiAdapter, dxgiAdapterTmp; // �f�o�C�X�����擾���邽�߂̃C���^�[�t�F�C�X
		int adapterIndex = 0; // �񋓂���f�o�C�X�̃C���f�b�N�X
		SIZE_T maxDedicatedVideoMemory = 0;//�g�b�v�̃r�f�I������
		//���[�v�ŒT��
		while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapterTmp) != DXGI_ERROR_NOT_FOUND) {
			//���擾
			DXGI_ADAPTER_DESC1 desc;
			dxgiAdapterTmp->GetDesc1(&desc);

			//TODO �A�_�v�^�A�I�v�V�����őI���ł���悤��

			//HardwareAdapter���g�p���� & �r�f�I����������葽������
			if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) && desc.DedicatedVideoMemory >= maxDedicatedVideoMemory) {
				dxgiAdapter = dxgiAdapterTmp;
				maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
			}
			++adapterIndex;
		}

		// �@�\���x��11�𖞂����n�[�h�E�F�A��������Ȃ��ꍇ�AWARP�f�o�C�X�̍쐬�����݂�.
		if (!dxgiAdapter) {
			if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)))) {
				return false;//�������߂����[(BNBN)
			}
		}

		//�g�p����@�\���x��
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};		

		//D3D12�f�o�C�X�̍쐬
		for (auto fuatureLevel : featureLevels) {
			if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), fuatureLevel, IID_PPV_ARGS(&m_d3dDevice)))) {
				break;
			}
		}

		{
			// �R�}���h�L���[���쐬
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			auto hr = m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
			if (FAILED(hr)) {
				return false;
			}
		}

		// �X���b�v�`�F�[�����쐬
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

		//IDXGISwapChain3�̃C���^�[�t�F�[�X���擾�B
		swapChainTmp.As(&m_swapChain);
		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		//RTV�p�̃f�X�N���v�^�q�[�v�쐬
		m_rtvDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_COUNT, false, m_rtvDescriptorHeap);

		//RTV�쐬
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (int i = 0; i < FRAME_COUNT; ++i) {
			auto hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
			if (FAILED(hr)) {
				return false;
			}
			m_d3dDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += m_rtvDescriptorSize;
		}

		//DSV�p�̃f�X�N���v�^�q�[�v�쐬
		m_dsvDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false, m_dsvDescriptorHeap);

		//DSV�쐬
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
				//�[�x�X�e���V���o�b�t�@�̍쐬�Ɏ��s�B
				return false;
			}
			//�f�B�X�N���v�^���쐬
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

			m_d3dDevice->CreateDepthStencilView(
				m_depthStencilBuffer.Get(), nullptr, dsvHandle
			);
		}*/

		// �R�}���h�A���P�[�^���쐬����.
		for (int i = 0; i < FRAME_COUNT; ++i) {
			if (FAILED(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])))) {
				return false;
			}
			wchar_t name[32];
			swprintf_s(name, L"m_commandAllocator[%d]", i);
			m_commandAllocator[i]->SetName(name);
		}

		// �R�}���h���X�g���쐬����.
		//�X���b�h�����n��Ƃ�
		if (FAILED(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_currentBackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)))) {
			return false;
		}
		//�R�}���h���X�g�͊J����Ă����Ԃō쐬�����̂ŁA�����������B
		if (FAILED(m_commandList->Close())) {
			return false;
		}

		// �t�F���X�ƃt�F���X�C�x���g���쐬����.
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
			//���C�������_�����O�^�[�Q�b�g���쐬�B
			float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
			if (m_mainRenderTarget.Create(
				initParam.frameBufferWidth,
				initParam.frameBufferHeight,
				1,
				1,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_D32_FLOAT,
				clearColor) == false) {
				TK_ASSERT(false, "���C�������_�����O�^�[�Q�b�g�̍쐬�Ɏ��s���܂����B");
				return false;
			}

			//CBR_SVR�̃f�B�X�N���v�^�̃T�C�Y���擾�B
			m_cbrSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			//�p�C�v���C���X�e�[�g���������B
			CPipelineStatesDx12::Init();

			//�|�X�g�G�t�F�N�g�̏������B
			m_postEffect.Init(initParam.graphicsConfing);

			m_copyFullScreenSprite.Init(
				&m_mainRenderTarget.GetRenderTargetTexture(),
				static_cast<float>(initParam.frameBufferWidth),
				static_cast<float>(initParam.frameBufferHeight));
			//�r���[�|�[�g���������B
			m_viewport.TopLeftX = 0;
			m_viewport.TopLeftY = 0;
			m_viewport.Width = static_cast<float>(initParam.frameBufferWidth);
			m_viewport.Height = static_cast<float>(initParam.frameBufferHeight);
			m_viewport.MinDepth = D3D12_MIN_DEPTH;
			m_viewport.MaxDepth = D3D12_MAX_DEPTH;

			//�V�U�����O��`���������B
			m_scissorRect.left = 0;
			m_scissorRect.top = 0;
			m_scissorRect.right = initParam.frameBufferWidth;
			m_scissorRect.bottom = initParam.frameBufferHeight;

			//�����_�����O�R���e�L�X�g�̍쐬�B
			auto giFactry = g_engine->GetGraphicsInstanceFactory();
			m_renderContext = giFactry->CreateRenderContext();
			auto& rcDx12 = m_renderContext->As<CRenderContextDx12>();
			rcDx12.SetCommandList(m_commandList);
			*/
	}

	void DX12Test::Release(){
		WaitForGpu();
		CloseHandle(m_fenceEvent);
		//Report();
	}

	void DX12Test::Report() {
#ifndef DW_MASTER
		//Release�Y��̏o��
		ID3D12DebugDevice* debugInterface;
		if (SUCCEEDED(m_d3dDevice->QueryInterface(&debugInterface)))
		{
			HRESULT hr = debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			debugInterface->Release();
		}
#endif
	}

	void DX12Test::Render() {
		//�O�t���[���̕`�抮����҂�
		if (!WaitForPreviousFrame()) {
			return;
		}

		//���ꂩ��g���R�}���h���X�g�E�A���P�[�^�����Z�b�g���Ďg�p�\��
		if (FAILED(m_commandAllocator[m_currentBackBufferIndex]->Reset())) {
			return;
		}
		if (FAILED(m_commandList->Reset(m_commandAllocator[m_currentBackBufferIndex].Get(), nullptr))) {
			return;
		}

		//���\�[�X�o���A��ݒ�
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		//�����_�[�^�[�Q�b�g��ݒ�
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		//�����_�[�^�[�Q�b�g��h��Ԃ�
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

		//���\�[�X�o���A��ݒ�
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		//�R�}���h���X�g�����
		if (FAILED(m_commandList->Close())) {
			return;
		}

		//�R�}���h���X�g���s
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//�X���b�v�`�F�C��
		if (FAILED(m_swapChain->Present(1, 0))) {
			return;
		}

		//�t�F���X�̃C���N�������g
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