#include "DWstdafx.h"
#include "PreRenderRender.h"

namespace DemolisherWeapon {

	void PreRenderRender::Render() {
		//�X�N���[���ԍ���ݒ�
		GetGraphicsEngine().SetTargetScreenNum(m_cameraNum);
		//3D�`��O�������s
		GetEngine().GetGameObjectManager().Pre3DRender(m_cameraNum);
	}

}