#include "DWstdafx.h"
#include "PreRenderRender.h"

namespace DemolisherWeapon {

	void PreRenderRender::Render() {
		//3D描画前処理実行
		GetEngine().GetGameObjectManager().Pre3DRender(m_cameraNum);
	}

}