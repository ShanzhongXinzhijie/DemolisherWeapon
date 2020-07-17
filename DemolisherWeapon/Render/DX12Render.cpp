#include "DWstdafx.h"
#include "DX12Render.h"
#include "GraphicsAPI/DirectX12/DX12Test.h"

#ifdef DW_DX12
namespace DemolisherWeapon {

	void DX12Render::Render() {
		m_dx12ref->Render();
	}

}
#endif