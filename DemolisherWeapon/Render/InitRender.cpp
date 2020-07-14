#include "DWstdafx.h"
#include "InitRender.h"

namespace DemolisherWeapon {
	void InitRender::Render() {
#ifndef DW_DX12_TEMPORARY
		//ブレンドステート初期化
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().NonPremultiplied(), nullptr, 0xFFFFFFFF);
#endif
		//スクリーン番号を設定
		GetGraphicsEngine().SetTargetScreenNum(-1);

		/*
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
		*/

		//バックバッファをクリア
		GetGraphicsEngine().ClearBackBuffer();

		//3D用のビューポートにする
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetGraphicsEngine().Get3DFrameBuffer_W(), GetGraphicsEngine().Get3DFrameBuffer_H());
	}
}