#include "DWstdafx.h"
#include "InitRender.h"

namespace DemolisherWeapon {
	void InitRender::Render() {
		//�u�����h�X�e�[�g������
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().NonPremultiplied(), nullptr, 0xFFFFFFFF);
	}
}