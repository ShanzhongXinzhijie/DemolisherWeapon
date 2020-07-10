#pragma once

namespace DemolisherWeapon {
	struct InitEngineParameter;

	class DX12Test
	{
	/*
	//�V���O���g��
	private:
		DX12Test(){}
		~DX12Test() {
			Release();
		}
	public:
		DX12Test(const DX12Test&) = delete;
		DX12Test& operator=(const DX12Test&) = delete;
		DX12Test(DX12Test&&) = delete;
		DX12Test& operator=(DX12Test&&) = delete;
	public:
		//�C���X�^���X���擾
		static DX12Test& GetIns()
		{
			if (instance == nullptr) {
				instance = new DX12Test;
			}
			return *instance;
		}
		//�C���X�^���X�̍폜
		static void DeleteIns() {
			if (instance) {
				delete instance; instance = nullptr;
			}
		}
	private:
		static inline DX12Test* instance = nullptr;
	//
	*/

	public:
		DX12Test() = default;
		~DX12Test() {
			Release();
		}

		/// <summary>
		/// DiretX12�̏�����
		/// </summary>
		bool Init(HWND hWnd, const InitEngineParameter& initParam);

		/// <summary>
		/// DirectX12�̏I������
		/// </summary>
		void Release();

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
		void Render();

		//�O�t���[���̕`�抮����҂�
		bool WaitForPreviousFrame()
		{
			if (m_fence->GetCompletedValue() < m_fenceValue[m_currentBackBufferIndex]) {
				if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
					return false;
				}
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
			m_fenceValue[m_currentBackBufferIndex]++;
			m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
			return true;
		}

		//GPU�̂��ׂĂ̏����̏I����҂�
		bool WaitForGpu()
		{
			m_fenceValue[m_currentBackBufferIndex]++;
			if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
				return false;
			}
			if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
				return false;
			}
			WaitForSingleObject(m_fenceEvent, INFINITE);
			return true;
		}

		/// <summary>
		/// D3D12�f�o�C�X���擾
		/// </summary>
		ID3D12Device* GetD3DDevice()
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

	private:
		static constexpr int FRAME_COUNT = 2;

		Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		int m_currentBackBufferIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
		UINT m_rtvDescriptorSize = 0;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
		UINT m_dsvDescriptorSize = 0;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent;
		UINT64 m_fenceValue[FRAME_COUNT];
	};

}
