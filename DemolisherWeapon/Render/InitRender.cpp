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
	}
}