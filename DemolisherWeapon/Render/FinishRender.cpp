#include "DWstdafx.h"
#include "FinishRender.h"

namespace DemolisherWeapon {

	void SuperUltraSeriousRealTrueFinishRender::Render() {
		//バックバッファを表へ
		GetEngine().GetGraphicsEngine().SwapBackBuffer();
	}

}