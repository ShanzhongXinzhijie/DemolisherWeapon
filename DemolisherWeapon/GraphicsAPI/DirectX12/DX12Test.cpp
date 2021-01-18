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

		struct ConstantBufferData {
			CMatrix mWorld;
			CMatrix mView;
			CMatrix mProj;
		};
		ConstantBufferData m_cbData;
		ConstantBuffer<ConstantBufferData> m_cb;
	};	

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
			ComPtr<ID3D12Debug1> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				debugController->SetEnableGPUBasedValidation(true);
				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}

			//Device Removed Extended Data�̗L����
			ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> d3dDredSettings1;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDredSettings1)))) {
				// Turn on AutoBreadcrumbs and Page Fault reporting
				d3dDredSettings1->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				d3dDredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				d3dDredSettings1->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
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
		swapChainDesc.Width = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W());
		swapChainDesc.Height = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H());
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		//�t���X�N���[���ݒ�
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDescPtr = nullptr;
		if (!initParam.isWindowMode) {
			fullscreenDescPtr = &fullscreenDesc;//�g��		

			auto refleshRate = Util::GetRefreshRate(hWnd);//���t���b�V�����[�g���擾
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

			CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto hr = m_d3dDevice->CreateCommittedResource(
				&prop,
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
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

			m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, dsvHandle);
		}

		//CBV_SRV_UAV�p�̃f�X�N���v�^�q�[�v�쐬
		m_srvsDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CBV_SRV_UAV_MAXNUM, false, m_srvsDescriptorHeap);
		//�_�~�[�n���h���쐬
		m_srvIndex++;
		m_srvsDammyCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

		//�`��p
		m_drawSRVsDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DRW_SRVS_DESC_NUM, true, m_drawSRVsDescriptorHeap);

		//�T���v���[�p�̃f�X�N���v�^�q�[�v�쐬
		m_samplerDescriptorSize = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SAMPLER_MAXNUM, true, m_samplerDescriptorHeap);

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
		m_commandList->SetName(L"DWCommandList");

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
		m_currentFenceValue = 0;

		//���C�g���[�V���O		
		m_rayTraceEngine = new RayTracingEngine;

		return true;
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

	bool DX12Test::RenderInit() {
		if (m_isInitTest)return true;

		//�V�F�[�_�̃��[�h
		m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);
		
		//���[�g�V�O�l�`���̍쐬
		D3D12_DESCRIPTOR_RANGE descRange[] = {
			CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0),//0�Ԃɂ�����?
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

		//�p�C�v���C���X�e�[�g�I�u�W�F�N�g�̍쐬
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

		//�v���~�e�B�u
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

		//�e�N�X�`��
		//m_texture2 = CreateTexture(L"utc_all2.dds");		
		//m_texture = CreateTexture(L"utc_all2.dds");
		const TextueData* return_textureData;
		TextureFactory::GetInstance().Load(L"utc_all2.dds", &return_textureData, true);//TODO ����̂��ƃe�N�X�`�����[�h�s�\?
		m_texture = *return_textureData;

		//���f��
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
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
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

		m_rayTraceTestModel[0] = new CModel;
		m_rayTraceTestModel[0]->LoadTkmFile("Assets/modelData/background.tkm");
		m_rayTraceTestModel[0]->CreateMeshParts();
		m_rayTraceTestModel[1] = new CModel;
		m_rayTraceTestModel[1]->LoadTkmFile("Assets/modelData/unityChan.tkm");
		m_rayTraceTestModel[1]->CreateMeshParts();

		//�`��q�[�v�ɃR�s�[
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
		m_camera.SetFar(10000.0f);
		m_camPos = { 81.2955322f, 105.524132f,  -98.4609833f };
		m_camTgt = { -34.3155823f, -16.9616947f, 104.190804f };

		{			
			//�o�C�A�X�s��擾
			CMatrix mBiasScr;
			CoordinateSystemBias::GetBias(m_rayTraceTestModelMatWorld, mBiasScr, enFbxUpAxisZ, enFbxRightHanded);
			m_rayTraceTestModelMatWorld.Mul(mBiasScr, m_rayTraceTestModelMatWorld);

			m_rayTraceEngine->RegisterModel(*m_rayTraceTestModel[0], &m_rayTraceTestModelMatWorld);

			int i = 0;
			for (auto& m : m_rayTraceTestModelMatUnity) {
				m = m_rayTraceTestModelMatWorld;
				mBiasScr.MakeTranslation({ 10.0f * i,0.0f,100.0f });
				m.Mul(m, mBiasScr);

				m_rayTraceEngine->RegisterModel(*m_rayTraceTestModel[1], &m_rayTraceTestModelMatUnity[i]);

				i++;
			}

			m_rayTraceEngine->Update(m_commandList.Get());
		}

		//����������
		m_isInitTest = true;
		return true;
	}

	void DX12Test::Render() {
		//RenderInit();

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

		{
			//���\�[�X�o���A��ݒ�
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_commandList->ResourceBarrier(1, &barrier);
		}

		//�����_�[�^�[�Q�b�g��ݒ�
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		//�����_�[�^�[�Q�b�g��h��Ԃ�
		CVector4 clearColor = { 1.0f, 0.2f, 0.4f, 1.0f };
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor.v, 0, nullptr);
		//�f�v�X�̃N���A
		GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//������
		RenderInit();

		//�f�B�X�N���v�^�q�[�v�̐ݒ�
		//ID3D12DescriptorHeap* heapList[] = { m_srvsDescriptorHeap.Get() };
		//m_commandList->SetDescriptorHeaps(_countof(heapList), heapList);

		//�r���[�|�[�g�ݒ�
		SetViewport(0.0f, 0.0f, GetGraphicsEngine().Get3DFrameBuffer_W(), GetGraphicsEngine().Get3DFrameBuffer_H());

		//�l�p�`�̕`��
		//{
		//	//���_�̍X�V
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
		
		{
			//�J����
			if (GetKeyInput('W')) {
				m_camPos += m_camera.GetFront() * 3.0f;
			}
			if (GetKeyInput('S')) {
				m_camPos += m_camera.GetFront() * -3.0f;
			}
			if (GetKeyInput('A')) {
				m_camPos += m_camera.GetLeft() * 3.0f;
			}
			if (GetKeyInput('D')) {
				m_camPos += m_camera.GetLeft() * -3.0f;
			}
			if (GetKeyInput(VK_SPACE)) {
				m_camPos += m_camera.GetUp() * 3.0f;
			}
			if (GetKeyInput(VK_LCONTROL)) {
				m_camPos += m_camera.GetUp() * -3.0f;
			}

			CVector2 move;
			if (GetKeyInput('E')) {
				move.x += 0.02f;
			}
			if (GetKeyInput('Q')) {
				move.x += -0.02f;
			}
			if (GetKeyInput('R')) {
				move.y += 0.02f;
			}
			if (GetKeyInput('F')) {
				move.y += -0.02f;
			}
			CQuaternion rot;
			rot.SetRotation(CVector3::Up(), move.x);
			rot.Multiply(m_camTgt);
			rot.SetRotation(m_camera.GetLeft(), move.y);
			rot.Multiply(m_camTgt);

			m_camera.SetPos(m_camPos);
			m_camera.SetTarget(m_camPos + m_camTgt);
		}

		{
			//���f���ړ�
			CVector3 move;
			if (GetKeyInput(VK_UP)) {
				move += m_camera.GetFront() * 3.0f;
			}
			if (GetKeyInput(VK_DOWN)) {
				move += m_camera.GetFront() * -3.0f;
			}
			if (GetKeyInput(VK_LEFT)) {
				move += m_camera.GetLeft() * 3.0f;
			}
			if (GetKeyInput(VK_RIGHT)) {
				move += m_camera.GetLeft() * -3.0f;
			}

			CMatrix m; m.MakeTranslation(move);
			m_rayTraceTestModelMatUnity[0].Mul(m_rayTraceTestModelMatUnity[0], m);
			m_rayTraceEngine->UpdateTLAS(m_commandList.Get());
		}

		//���C�g��
		m_rayTraceEngine->Dispatch(m_commandList.Get());

		//{
		//	static float rot = 0;
		//	rot += CMath::RadToDeg(0.001f);
		//	m_meshTest->m_cbData.mWorld = m_rayTraceTestModelMat[1];
		//	m_meshTest->m_cbData.mView = GetMainCamera()->GetViewMatrix();
		//	m_meshTest->m_cbData.mProj = GetMainCamera()->GetProjMatrix();
		//	m_meshTest->m_cb.Update(&m_meshTest->m_cbData);

		//	//���f���̕`��
		//	m_commandList->SetPipelineState(m_meshTest->m_pso.Get());
		//	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

		//	ID3D12DescriptorHeap* heapList[] = { m_drawSRVsDescriptorHeap.Get() };
		//	m_commandList->SetDescriptorHeaps(_countof(heapList), heapList);
		//	m_commandList->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_drawSRVsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_drawSRVsDescriptorSize));
		
		//	//m_meshTest->m_mesh.Draw(1);
		//	m_rayTraceTestModel[1].Draw(1);
		//}

		{
			//���\�[�X�o���A��ݒ�
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			m_commandList->ResourceBarrier(1, &barrier);
		}

		//�R�}���h���X�g�����
		if (FAILED(m_commandList->Close())) {
			return;
		}

		//�R�}���h���X�g���s
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//�X���b�v�`�F�C��
		HRESULT hr = m_swapChain->Present(GetGraphicsEngine().GetUseVSync() ? 1 : 0, 0);
		if (FAILED(hr)) {
			hr = m_d3dDevice->GetDeviceRemovedReason();
			std::abort();
		}

		//�t�F���X�̃C���N�������g
		m_currentFenceValue++;
		m_fenceValue[m_currentBackBufferIndex] = m_currentFenceValue;
		//�t�F���X�̒l�ύX
		if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
			return;
		}
	}

	bool DX12Test::WaitForPreviousFrame() {
		//�҂�
		DWORD hr;
		if (m_fence->GetCompletedValue() < m_fenceValue[m_currentBackBufferIndex]) {
			if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
				return false;
			}
			hr = WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		//�`��o�b�t�@����ւ�
		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
		return true;
	}

	bool DX12Test::WaitForGpu() {
		//�ӂ₷
		m_currentFenceValue++;
		m_fenceValue[m_currentBackBufferIndex] = m_currentFenceValue;
		//�t�F���X�̒l�ύX
		if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
			return false;
		}
		//�҂�
		if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
			return false;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
		return true;
	}

	void DX12Test::SetBackBufferToRenderTarget() {		
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
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier(1, &barrier);

		//�����_�[�^�[�Q�b�g��ݒ�
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);		
	}

	void DX12Test::ExecuteCommand() {
		//���\�[�X�o���A��ݒ�
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &barrier);

		//�R�}���h���X�g�����
		if (FAILED(m_commandList->Close())) {
			return;
		}

		//�R�}���h���X�g���s
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//�t�F���X�̃C���N�������g
		m_currentFenceValue++;
		m_fenceValue[m_currentBackBufferIndex] = m_currentFenceValue;

		//�t�F���X�̒l�ύX
		if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
			return;
		}
	}

	void DX12Test::SwapBackBuffer() {
		//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���B
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

	void DX12Test::RayTarcingCommit() {
		m_rayTraceEngine->Update(m_commandList.Get());
	}
}