#include "DWstdafx.h"
#include "PreRenderRender.h"

namespace DemolisherWeapon {

	void PreRenderRender::Render() {
		//3D•`‰æ‘Oˆ—Às
		GetEngine().GetGameObjectManager().Pre3DRender(m_cameraNum);
	}

}