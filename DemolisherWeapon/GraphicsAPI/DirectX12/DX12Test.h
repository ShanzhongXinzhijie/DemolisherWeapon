#pragma once
#include"GraphicsAPI/IGraphicsAPI.h"

namespace DemolisherWeapon {
	struct InitEngineParameter;

	class DX12Test : public IGraphicsAPI
	{
	public:
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
		bool WaitForPreviousFrame(){
			//�҂�
			if (m_fence->GetCompletedValue() < m_fenceValue[m_currentBackBufferIndex]) {
				if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
					return false;
				}
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
			//�`��o�b�t�@����ւ�
			m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
			return true;
		}

		//GPU�̂��ׂĂ̏����̏I����҂�
		bool WaitForGpu(){
			//�ӂ₷
			m_fenceValue[m_currentBackBufferIndex]++;
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

		/// <summary>
		/// D3D12�f�o�C�X���擾
		/// </summary>
		ID3D12Device* GetD3D12Device()
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

	private:
		static constexpr int FRAME_COUNT = 2;

		Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		int m_currentBackBufferIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
		UINT m_rtvDescriptorSize = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
		UINT m_dsvDescriptorSize = 0;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent;
		UINT64 m_fenceValue[FRAME_COUNT];

		D3D12_VIEWPORT m_viewport;//�r���[�|�[�g
		D3D12_RECT     m_scissorRect;//�V�U�[��`

		//�e�X�g�`��
		bool m_isInitTest = false;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;//���[�g�V�O�l�`��
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
		Shader m_vs, m_ps;
		CPrimitive m_square;
	};

}