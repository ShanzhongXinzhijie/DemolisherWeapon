#include "DWstdafx.h"
#include "PreRenderRender.h"

namespace DemolisherWeapon {

	void PreRenderRender::Render() {
		//3D�`��O�������s
		GetEngine().GetGameObjectManager().Pre3DRender(m_cameraNum);
	}

}