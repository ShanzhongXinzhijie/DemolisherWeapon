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

		//バックバッファをレンダーターゲットに
		GetGraphicsEngine().SetBackBufferToRenderTarget();

		//バックバッファをクリア
		GetGraphicsEngine().ClearBackBuffer();

		//3D用のビューポートにする
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetGraphicsEngine().Get3DFrameBuffer_W(), GetGraphicsEngine().Get3DFrameBuffer_H());
	}

	void InitRender2D::Render() {
		//2D用の設定にする
		GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());
	}
}