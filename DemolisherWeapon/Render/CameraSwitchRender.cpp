#include "DWstdafx.h"
#include "CameraSwitchRender.h"

namespace DemolisherWeapon {

	CameraSwitchRender::CameraSwitchRender()
	{
	}


	CameraSwitchRender::~CameraSwitchRender()
	{
	}

	void CameraSwitchRender::Render() {
		auto& camlist = GetEngine().GetGraphicsEngine().GetCameraManager().GetCameraList();		
		
#ifdef _DEBUG
		if (camlist.size() <= m_cameraNum) {
			MessageBox(NULL, "�J�����̐�������Ă܂���B(��ʕ���)", "Error", MB_OK);
			std::abort();
		}
#endif

		SetMainCamera(camlist[m_cameraNum]);

		//�V�F�[�_�ɑ���|�C���g���C�g�p�J�������W�X�V
		GetEngine().GetGraphicsEngine().GetLightManager().UpdateConstantBuffer();
	}
}