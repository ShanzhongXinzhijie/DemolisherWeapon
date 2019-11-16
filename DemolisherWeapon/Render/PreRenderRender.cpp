#include "DWstdafx.h"
#include "PreRenderRender.h"

namespace DemolisherWeapon {

	void PreRenderRender::Render() {
		//スクリーン番号を設定
		GetGraphicsEngine().SetTargetScreenNum(m_cameraNum);
		//3D描画前処理実行
		GetEngine().GetGameObjectManager().Pre3DRender(m_cameraNum);
	}

}