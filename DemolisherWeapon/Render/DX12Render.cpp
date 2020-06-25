#include "DWstdafx.h"
#include "DX12Render.h"
#include "DirectX12/d3dx12.h"

namespace DemolisherWeapon {

	void DX12Render::Render() {
		//�O�t���[���̕`�抮����҂�
		if (!WaitForPreviousFrame()) {
			return false;
		}

		//���ꂩ��g���R�}���h���X�g�E�A���P�[�^�����Z�b�g���Ďg�p�\��
		if (FAILED(commandAllocator[currentFrameIndex]->Reset())) {
			return false;
		}
		if (FAILED(commandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
			return false;
		}

		//���\�[�X�o���A��ݒ�
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		
		//�����_�[�^�[�Q�b�g��ݒ�
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += currentFrameIndex * rtvDescriptorSize;
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		//�����_�[�^�[�Q�b�g��h��Ԃ�
		const float clearColor[] = { 1.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		
		//���\�[�X�o���A��ݒ�
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetList[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		//�R�}���h���X�g�����
		if (FAILED(commandList->Close())) {
			return false;
		}

		//�R�}���h���X�g���s
		ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//�X���b�v�`�F�C��
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