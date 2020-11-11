#pragma once
#include"GraphicsAPI/IGraphicsAPI.h"

namespace DemolisherWeapon {
	struct InitEngineParameter;
	struct MeshTest;
	class RayTracingEngine;

	class DX12Test : public IGraphicsAPI
	{
	public:
		static constexpr int FRAME_COUNT = 2;
		static constexpr int CBV_SRV_UAV_MAXNUM = 1024;
		static constexpr int DRW_SRVS_DESC_NUM = 32;
		static constexpr int SAMPLER_MAXNUM = 16;

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
		ID3D12Device5* GetD3D12Device()
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
		/// SRVとかのダミーCPUディスクリプタハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetSrvsDescriptorDammyCPUHandle() {
			return m_srvsDammyCPUHandle; 
		}

		/// <summary>
		/// SRVを作成
		/// </summary>
		/// <param name="resource">参照するリソース</param>
		/// <returns>GPUディスクリプタハンドルとCPUディスクリプタハンドル</returns>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> CreateSRV(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* desc = nullptr) {
			m_srvIndex++;

			/*
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = m_textureDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = m_textureDesc.MipLevels;
			*/

			D3D12_CPU_DESCRIPTOR_HANDLE cpuH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

			//SAV作るぞー
			GetD3D12Device()->CreateShaderResourceView(resource, desc, cpuH);
			
			//Gpu側のハンドル取得
			return {
				CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize),
				cpuH
			};
		}

		/// <summary>
		/// 定数バッファを作成
		/// </summary>
		/// <param name="resource"></param>
		/// <returns></returns>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> CreateConstantBufferView(ID3D12Resource* resource, int allocSize) {
			m_srvIndex++;
			
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = resource->GetGPUVirtualAddress();
			desc.SizeInBytes = allocSize;

			D3D12_CPU_DESCRIPTOR_HANDLE cpuH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

			//CBVつくる
			GetD3D12Device()->CreateConstantBufferView(&desc, cpuH);
			
			//Gpu側のハンドル取得
			return {
				CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize),
				cpuH
			};
		}

		/// <summary>
		/// UAVを作成
		/// </summary>
		/// <param name="resource">参照するリソース</param>
		/// <returns>GPUディスクリプタハンドル</returns>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> CreateUAV(ID3D12Resource* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc) {
			m_srvIndex++;

			D3D12_CPU_DESCRIPTOR_HANDLE cpuH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize);

			//UAV作るぞー
			GetD3D12Device()->CreateUnorderedAccessView(resource, nullptr, &UAVDesc, cpuH);

			//Gpu側のハンドル取得
			return {
				CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_srvIndex, m_srvsDescriptorSize),
				cpuH
			};
		}

		/// <summary>
		/// SRVとかのディスクリプタサイズを取得
		/// </summary>
		/// <returns></returns>
		UINT GetSrvsDescriptorSize()const {
			return m_srvsDescriptorSize;
		}

		/// <summary>
		/// SRVとかのディスクリプタヒープ先頭を取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvsDescriptorHeapStart()const {
			return m_srvsDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}

		/// <summary>
		/// SRVとかのディスクリプタヒープ取得
		/// </summary>
		/// <returns></returns>
		ID3D12DescriptorHeap* GetSrvsDescriptorHeap()const {
			return m_srvsDescriptorHeap.Get();
		}

		/// <summary>
		/// サンプラーディスクリプタヒープ先頭を取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerDescriptorHeapStart()const {
			return m_samplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}

		/// <summary>
		/// サンプラーディスクリプタヒープ取得
		/// </summary>
		/// <returns></returns>
		ID3D12DescriptorHeap* GetSamplerDescriptorHeap()const {
			return m_samplerDescriptorHeap.Get();
		}

		/// <summary>
		/// 現フレームのレンダーターゲットを取得
		/// </summary>
		/// <returns></returns>
		ID3D12Resource* GetCurrentRenderTarget() {
			return m_renderTargets[m_currentBackBufferIndex].Get();
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
		Microsoft::WRL::ComPtr<ID3D12Device5> m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

		//スワップチェイン
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		int m_currentBackBufferIndex = 0;

		//レンダーターゲット
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
		UINT m_rtvDescriptorSize = 0;

		//デプスステンシル
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
		UINT m_dsvDescriptorSize = 0;

		//SRVとかのディスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvsDescriptorHeap;
		UINT m_srvsDescriptorSize = 0;
		int m_srvIndex = -1;
		D3D12_CPU_DESCRIPTOR_HANDLE m_srvsDammyCPUHandle;

		//描画用SRVディスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_drawSRVsDescriptorHeap;
		UINT m_drawSRVsDescriptorSize = 0;

		//サンプラーのディスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerDescriptorHeap;
		UINT m_samplerDescriptorSize = 0;

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_commandList;
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
		TextueData m_texture, m_texture2;

		//メッシュ描画のテスト
		MeshTest* m_meshTest = nullptr;

		//カメラ
		GameObj::PerspectiveCamera m_camera;
		CVector3 m_camPos,m_camTgt = CVector3::Front()*111.0f;

		//レイトレエンジン
		RayTracingEngine* m_rayTraceEngine = nullptr;
		CModel m_rayTraceTestModel[2];
		CMatrix m_rayTraceTestModelMatWorld;
		CMatrix m_rayTraceTestModelMatUnity[100];
	};

}