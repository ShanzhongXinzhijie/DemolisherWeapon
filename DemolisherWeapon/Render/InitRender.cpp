#include "DWstdafx.h"
#include "InitRender.h"

namespace DemolisherWeapon {
	void InitRender::Render() {
#ifdef DW_DX11
		//�u�����h�X�e�[�g������
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().NonPremultiplied(), nullptr, 0xFFFFFFFF);
#endif
		//�X�N���[���ԍ���ݒ�
		GetGraphicsEngine().SetTargetScreenNum(-1);

		//�o�b�N�o�b�t�@�������_�[�^�[�Q�b�g��
		GetGraphicsEngine().SetBackBufferToRenderTarget();

		//�o�b�N�o�b�t�@���N���A
		GetGraphicsEngine().ClearBackBuffer();

		//3D�p�̃r���[�|�[�g�ɂ���
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetGraphicsEngine().Get3DFrameBuffer_W(), GetGraphicsEngine().Get3DFrameBuffer_H());
	}

	void InitRender2D::Render() {
		//2D�p�̐ݒ�ɂ���
		GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());
	}
}