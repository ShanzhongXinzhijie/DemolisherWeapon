#include "DWstdafx.h"
#include "FinishRender.h"

namespace DemolisherWeapon {

	void SuperUltraSeriousRealTrueFinishRender::Render() {
		//�R�}���h���s
		GetGraphicsEngine().ExecuteCommand();
		//�o�b�N�o�b�t�@��\��
		GetEngine().GetGraphicsEngine().SwapBackBuffer();

		GetGraphicsEngine().ExecuteCommandDirectXTK();
	}

}