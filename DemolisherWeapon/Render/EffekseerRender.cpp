#include "DWstdafx.h"
#include "EffekseerRender.h"

namespace DemolisherWeapon {

	void EffekseerRender::Render() {
		//GPU�C�x���g�̊J�n
		GetGraphicsEngine().BeginGPUEvent(L"EffekseerRender");

		//�����_�[�^�[�Q�b�g�Ƃ��ݒ�		
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
			1,
			&GetEngine().GetGraphicsEngine().GetFRT().GetRTV(),
			GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
		);

		//�`��
		GetEngine().GetEffekseer().Draw();

		//�����_�[�^�[�Q�b�g����
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//GPU�C�x���g�̏I��
		GetGraphicsEngine().EndGPUEvent();
	}

}