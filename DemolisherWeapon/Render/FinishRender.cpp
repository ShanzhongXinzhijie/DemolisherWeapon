#include "DWstdafx.h"
#include "FinishRender.h"

namespace DemolisherWeapon {

	void SuperUltraSeriousRealTrueFinishRender::Render() {
		//コマンド実行
		GetGraphicsEngine().ExecuteCommand();
		//バックバッファを表へ
		GetEngine().GetGraphicsEngine().SwapBackBuffer();

		GetGraphicsEngine().ExecuteCommandDirectXTK();
	}

}