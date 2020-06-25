#include "DWstdafx.h"
#include "DX12Render.h"
#include "DirectX12/d3dx12.h"

namespace DemolisherWeapon {

	void DX12Render::Render() {
		//前フレームの描画完了を待つ
		if (!WaitForPreviousFrame()) {
			return false;
		}

		//これから使うコマンドリスト・アロケータをリセットして使用可能に
		if (FAILED(commandAllocator[currentFrameIndex]->Reset())) {
			return false;
		}
		if (FAILED(commandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
			return false;
		}

		//リソースバリアを設定
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		
		//レンダーターゲットを設定
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += currentFrameIndex * rtvDescriptorSize;
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		//レンダーターゲットを塗りつぶし
		const float clearColor[] = { 1.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		
		//リソースバリアを設定
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		//コマンドリストを閉じる
		if (FAILED(commandList->Close())) {
			return false;
		}

		//コマンドリスト実行
		ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//スワップチェイン
		if (FAILED(swapChain->Present(1, 0))) {
			return false;
		}

		fenceValue[currentFrameIndex] = masterFenceValue;
		if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
			return false;
		}
		++masterFenceValue;
		
		if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
			return false;
		}
	}

	bool DX12Render::WaitForPreviousFrame()
	{
		if (fence->GetCompletedValue() < fenceValue[currentFrameIndex]) {
			if (FAILED(fence->SetEventOnCompletion(fenceValue[currentFrameIndex], fenceEvent))) {
				return false;
			}
			WaitForSingleObject(fenceEvent, INFINITE);
		}
		++fenceValue[currentFrameIndex];
		currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
		return true;
	}

	bool DX12Render::WaitForGpu()
	{
		++fenceValue[currentFrameIndex];
		if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
			return false;
		}
		if (FAILED(fence->SetEventOnCompletion(fenceValue[currentFrameIndex], fenceEvent))) {
			return false;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
		return true;
	}

}