#include "DWstdafx.h"
#include "FinishRender.h"

namespace DemolisherWeapon {

	void SuperUltraSeriousRealTrueFinishRender::Render() {
		//コマンド実行
		GetGraphicsEngine().ExecuteCommand();
		//バックバッファを表へ
		GetGraphicsEngine().SwapBackBuffer();

		GetGraphicsEngine().ExecuteCommandDirectXTK();
	}

}