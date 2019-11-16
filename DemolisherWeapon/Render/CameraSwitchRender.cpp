#include "DWstdafx.h"
#include "CameraSwitchRender.h"

namespace DemolisherWeapon {
	
	void CameraSwitchRender::Render() {
		auto& camlist = GetGraphicsEngine().GetCameraManager().GetCameraList();		
		
#ifndef DW_MASTER
		if (camlist.size() <= m_cameraNum) {
			MessageBox(NULL, "�J�����̐�������Ă܂���B(��ʕ���)", "Error", MB_OK);
			std::abort();
		}
#endif
		//���C���J�����ύX
		SetMainCamera(camlist[m_cameraNum]);

		//�V�F�[�_�ɑ���|�C���g���C�g�p�J�������W�X�V
		GetGraphicsEngine().GetLightManager().UpdateConstantBuffer();
	}

}