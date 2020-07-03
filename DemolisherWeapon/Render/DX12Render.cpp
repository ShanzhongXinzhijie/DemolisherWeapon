#include "DWstdafx.h"
#include "DX12Render.h"
#include "DirectX12/DX12Test.h"

namespace DemolisherWeapon {

	void DX12Render::Render() {
		DX12Test::GetIns().Render();
	}
}