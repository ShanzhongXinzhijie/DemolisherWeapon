#pragma once
#include"GraphicsAPI/IGraphicsAPI.h"

namespace DemolisherWeapon {
	struct InitEngineParameter;

	class DX12Test : public IGraphicsAPI
	{
	public:
		DX12Test() = default;

		/// <summary>
		/// DiretX12の初期化
		/// </summary>
		bool Init(HWND hWnd, const InitEngineParameter& initParam)override;

		/// <summary>
		/// DirectX12の終了処理
		/// </summary>
		void Release()override;

		/// <summary>
		/// フレームバッファサイズの変更(再設定)
		/// </summary>
		void ChangeFrameBufferSize()override {};

		/// <summary>
		/// デバッグ用レポートの出力
		/// </summary>
		void Report();

		/// <summary>
		/// ディスクリプタヒープの作成
		/// </summary>
		/// <param name="type"></param>
		/// <param name="numDescriptors"></param>
		/// <param name="isShaderVisible"></param>
		/// <param name="descriptorHeap"></param>
		/// <returns></returns>
		UINT CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool isShaderVisible, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap);

		//テストレンダ
		bool RenderInit();
		void Render();

		//コマンドを実行する
		void ExecuteCommand()override;

		//前フレームの描画完了を待つ
		bool WaitForPreviousFrame(){
			//待つ
			if (m_fence->GetCompletedValue() < m_fenceValue[m_currentBackBufferIndex]) {
				if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
					return false;
				}
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
			//描画バッファ入れ替え
			m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
			return true;
		}

		//GPUのすべての処理の終わりを待つ
		bool WaitForGpu(){
			//ふやす
			m_fenceValue[m_currentBackBufferIndex]++;
			//フェンスの値変更
			if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue[m_currentBackBufferIndex]))) {
				return false;
			}
			//待つ
			if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue[m_currentBackBufferIndex], m_fenceEvent))) {
				return false;
			}
			WaitForSingleObject(m_fenceEvent, INFINITE);
			return true;
		}

		/// <summary>
		/// D3D12デバイスを取得
		/// </summary>
		ID3D12Device* GetD3D12Device()
		{
			return m_d3dDevice.Get();
		}

		/// <summary>
		/// コマンドキューを取得
		/// </summary>
		ID3D12CommandQueue* GetCommandQueue()
		{
			return m_commandQueue.Get();
		}

		/// <summary>
		/// コマンドリストを取得
		/// </summary>
		ID3D12GraphicsCommandList* GetCommandList()
		{
			return m_commandList.Get();
		}

		/// <summary>
		/// バックバッファのクリア
		/// </summary>
		void ClearBackBuffer()override
		{
			//バックバッファを灰色で塗りつぶす。
			float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
			GetCommandList()->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);
			//デプスのクリア
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}

		/// <summary>
		/// バックバッファをレンダーターゲットにするなど
		/// </summary>
		void SetBackBufferToRenderTarget()override;

		/// <summary>
		/// バックバッファのスワップなど
		/// </summary>
		void SwapBackBuffer()override;

		/// <summary>
		/// ビューポートの設定
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

		D3D12_VIEWPORT m_viewport;//ビューポート
		D3D12_RECT     m_scissorRect;//シザー矩形

		//テスト描画
		bool m_isInitTest = false;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;//ルートシグネチャ
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;//パイプラインステートオブジェクト
		Shader m_vs, m_ps;
		CPrimitive m_square;
	};

}