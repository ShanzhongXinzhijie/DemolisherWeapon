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
			MessageBox(NULL, "カメラの数が足りてません。(画面分割)", "Error", MB_OK);
			std::abort();
		}
#endif

		SetMainCamera(camlist[m_cameraNum]);

		//シェーダに送るポイントライト用カメラ座標更新
		GetEngine().GetGraphicsEngine().GetLightManager().UpdateConstantBuffer();
	}
}