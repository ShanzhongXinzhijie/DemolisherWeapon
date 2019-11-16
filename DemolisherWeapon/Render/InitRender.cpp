#include "DWstdafx.h"
#include "InitRender.h"

namespace DemolisherWeapon {
	void InitRender::Render() {
		//ブレンドステート初期化
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().NonPremultiplied(), nullptr, 0xFFFFFFFF);
		//スクリーン番号を設定
		GetGraphicsEngine().SetTargetScreenNum(-1);
	}
}