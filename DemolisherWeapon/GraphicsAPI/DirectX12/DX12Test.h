#pragma once
#include"GraphicsAPI/IGraphicsAPI.h"
#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {
	struct InitEngineParameter;
	struct MeshTest;
	class RayTracingEngine;
	class CModel;

	class DX12Test : public IGraphicsAPI
	{
	public:
		static constexpr int FRAME_COUNT = 2;
		static constexpr int CBV_SRV_UAV_MAXNUM = 1024;
		static constexpr int DRW_SRVS_DESC_NUM = 32;
		static constexpr int SAMPLER_MAXNUM = 16;

		DX12Test() = default;

		/// <summary>
		/// DiretX12�̏�����
		/// </summary>
		bool Init(HWND hWnd, const InitEngineParameter& initParam)override;

		/// <summary>
		/// DirectX12�̏I������
		/// </summary>
		void Release()override;

		/// <summary>
		/// �t���[���o�b�t�@�T�C�Y�̕ύX(�Đݒ�)
		/// </summary>
		void ChangeFrameBufferSize()override {};

		/// <summary>
		/// �f�o�b�O�p���|�[�g�̏o��
		/// </summary>
		void Report();

		/// <summary>
		/// �f�B�X�N���v�^�q�[�v�̍쐬
		/// </summary>
		/// <param name="type"></param>
		/// <param name="numDescriptors"></param>
		/// <param name="isShaderVisible"></param>
		/// <param name="descriptorHeap"></param>
		/// <returns></returns>
		UINT CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool isShaderVisible, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap);

		//�e�X�g�����_
		bool RenderInit();
		void Render();

		//�R�}���h�����s����
		void ExecuteCommand()override;

		//�O�t���[���̕`�抮����҂�
		bool WaitForPreviousFrame();

		//GPU�̂��ׂĂ̏����̏I����҂�
		bool WaitForGpu();

		/// <summary>
		/// D3D12�f�o�C�X���擾
		/// </summary>
		ID3D12Device5* GetD3D12Device()
		{
			return m_d3dDevice.Get();
		}

		/// <summary>
		/// �R�}���h�L���[���擾
		/// </summary>
		ID3D12CommandQueue* GetCommandQueue()
		{
			return m_commandQueue.Get();
		}

		/// <summary>
		/// �R�}���h���X�g���擾
		/// </summary>
		ID3D12GraphicsCommandList* GetCommandList()
		{
			return m_commandList.Get();
		}
		ID3D12GraphicsCommandList4* GetCommandList4()
		{
			return m_commandList.Get();
		}

		/// <summary>
		/// SRV�Ƃ��̃_�~�[CPU�f�B�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetSrvsDescriptorDammyCPUHandle() {
			return m_srvsDammyCPUHandle; 
		}

		/// <summary>
		/// SRV���쐬
		/// </summary>
		/// <param name="resource">�Q�Ƃ��郊�\�[�X</param>
		/// <returns>GPU�f�B�X�N���v�^�n���h����CPU�f�B�X�N���v�^�n���h��</returns>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> CreateSRV(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* desc = nullptr) {
			m_srvIndex++;

			D3D12_CPU_DESCRIPTOR_HANDLE cpuH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

			//SAV��邼�[
			GetD3D12Device()->CreateShaderResourceView(resource, desc, cpuH);
			
			//Gpu���̃n���h���擾
			return {
				CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize),
				cpuH
			};
		}

		/// <summary>
		/// �萔�o�b�t�@���쐬
		/// </summary>
		/// <param name="resource"></param>
		/// <returns></returns>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> CreateConstantBufferView(ID3D12Resource* resource, int allocSize) {
			m_srvIndex++;
			
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = resource->GetGPUVirtualAddress();
			desc.SizeInBytes = allocSize;

			D3D12_CPU_DESCRIPTOR_HANDLE cpuH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

			//CBV����
			GetD3D12Device()->CreateConstantBufferView(&desc, cpuH);
			
			//Gpu���̃n���h���擾
			return {
				CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize),
				cpuH
			};
		}

		/// <summary>
		/// UAV���쐬
		/// </summary>
		/// <param name="resource">�Q�Ƃ��郊�\�[�X</param>
		/// <returns>GPU�f�B�X�N���v�^�n���h��</returns>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> CreateUAV(ID3D12Resource* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc) {
			m_srvIndex++;

			D3D12_CPU_DESCRIPTOR_HANDLE cpuH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

			//UAV��邼�[
			GetD3D12Device()->CreateUnorderedAccessView(resource, nullptr, &UAVDesc, cpuH);

			//Gpu���̃n���h���擾
			return {
				CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize),
				cpuH
			};
		}

		/// <summary>
		/// SRV�Ƃ��̃f�B�X�N���v�^�T�C�Y���擾
		/// </summary>
		/// <returns></returns>
		UINT GetSrvsDescriptorSize()const {
			return m_srvsDescriptorSize;
		}

		/// <summary>
		/// SRV�Ƃ��̃f�B�X�N���v�^�q�[�v�擪���擾
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvsDescriptorHeapStart()const {
			return m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}

		/// <summary>
		/// SRV�Ƃ��̃f�B�X�N���v�^�q�[�v�擾
		/// </summary>
		/// <returns></returns>
		ID3D12DescriptorHeap* GetSrvsDescriptorHeap()const {
			return m_srvsDescriptorHeap.Get();
		}

		/// <summary>
		/// �T���v���[�f�B�X�N���v�^�q�[�v�擪���擾
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerDescriptorHeapStart()const {
			return m_samplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}

		/// <summary>
		/// �T���v���[�f�B�X�N���v�^�q�[�v�擾
		/// </summary>
		/// <returns></returns>
		ID3D12DescriptorHeap* GetSamplerDescriptorHeap()const {
			return m_samplerDescriptorHeap.Get();
		}

		/// <summary>
		/// ���t���[���̃����_�[�^�[�Q�b�g���擾
		/// </summary>
		/// <returns></returns>
		ID3D12Resource* GetCurrentRenderTarget() {
			return m_renderTargets[m_currentBackBufferIndex].Get();
		}
		/// <summary>
		/// ���t���[���̔ԍ����擾
		/// </summary>
		/// <returns></returns>
		int GetCurrentBackBufferIndex()const {
			return m_currentBackBufferIndex;
		}

		/// <summary>
		/// �o�b�N�o�b�t�@�̃N���A
		/// </summary>
		void ClearBackBuffer()override
		{
			//�o�b�N�o�b�t�@���D�F�œh��Ԃ��B
			float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
			GetCommandList()->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);
			//�f�v�X�̃N���A
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}

		/// <summary>
		/// �o�b�N�o�b�t�@�������_�[�^�[�Q�b�g�ɂ���Ȃ�
		/// </summary>
		void SetBackBufferToRenderTarget()override;

		/// <summary>
		/// �o�b�N�o�b�t�@�̃X���b�v�Ȃ�
		/// </summary>
		void SwapBackBuffer()override;

		/// <summary>
		/// �r���[�|�[�g�̐ݒ�
		/// </summary>
		void SetViewport(float topLeftX, float topLeftY, float width, float height)override;

		/// <summary>
		/// ���C�g���G���W���̎擾
		/// </summary>
		/// <returns></returns>
		RayTracingEngine& GetRayTracingEngine() {
			return *m_rayTraceEngine;
		}
		/// <summary>
		/// ���C�g���G���W���̃W�I���g���o�^���m��
		/// </summary>
		void RayTarcingCommit();

	private:		
		Microsoft::WRL::ComPtr<ID3D12Device5> m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

		//�X���b�v�`�F�C��
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		int m_currentBackBufferIndex = 0;

		//�����_�[�^�[�Q�b�g
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
		UINT m_rtvDescriptorSize = 0;

		//�f�v�X�X�e���V��
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
		UINT m_dsvDescriptorSize = 0;

		//SRV�Ƃ��̃f�B�X�N���v�^�q�[�v
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvsDescriptorHeap;
		UINT m_srvsDescriptorSize = 0;
		int m_srvIndex = -1;
		D3D12_CPU_DESCRIPTOR_HANDLE m_srvsDammyCPUHandle;

		//�`��pSRV�f�B�X�N���v�^�q�[�v
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_drawSRVsDescriptorHeap;
		UINT m_drawSRVsDescriptorSize = 0;

		//�T���v���[�̃f�B�X�N���v�^�q�[�v
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerDescriptorHeap;
		UINT m_samplerDescriptorSize = 0;

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent;
		UINT64 m_fenceValue[FRAME_COUNT] = {};
		UINT64 m_currentFenceValue = 0;

		D3D12_VIEWPORT m_viewport;//�r���[�|�[�g
		D3D12_RECT     m_scissorRect;//�V�U�[��`



		//�e�X�g�`��
		bool m_isInitTest = false;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;//���[�g�V�O�l�`��
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
		Shader m_vs, m_ps;
		CPrimitive m_square;
		TextueData m_texture, m_texture2;

		//���b�V���`��̃e�X�g
		MeshTest* m_meshTest = nullptr;

		//�J����
		GameObj::PerspectiveCamera m_camera;
		CVector3 m_camPos,m_camTgt = CVector3::Front()*111.0f;

		//���C�g���G���W��
		RayTracingEngine* m_rayTraceEngine = nullptr;
		CModel* m_rayTraceTestModel[2];
		CMatrix m_rayTraceTestModelMatWorld;
		CMatrix m_rayTraceTestModelMatUnity[100];
	};

}