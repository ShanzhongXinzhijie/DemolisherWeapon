#include "DWstdafx.h"
#include "EffekseerRender.h"

namespace DemolisherWeapon {

	void EffekseerRender::Render() {
		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"EffekseerRender");

		//レンダーターゲットとか設定		
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
			1,
			&GetEngine().GetGraphicsEngine().GetFRT().GetRTV(),
			GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
		);

		//描画
		GetEngine().GetEffekseer().Draw();

		//レンダーターゲット解除
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//GPUイベントの終了
		GetGraphicsEngine().EndGPUEvent();
	}

}