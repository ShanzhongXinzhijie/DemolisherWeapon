#include "DWstdafx.h"
#include "InitRender.h"

namespace DemolisherWeapon {
	void InitRender::Render() {
#ifndef DW_DX12_TEMPORARY
		//�u�����h�X�e�[�g������
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().NonPremultiplied(), nullptr, 0xFFFFFFFF);
#endif
		//�X�N���[���ԍ���ݒ�
		GetGraphicsEngine().SetTargetScreenNum(-1);
	}
}