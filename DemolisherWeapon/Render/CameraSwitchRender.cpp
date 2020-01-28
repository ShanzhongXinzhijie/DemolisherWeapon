#include "DWstdafx.h"
#include "CameraSwitchRender.h"

namespace DemolisherWeapon {
	
	void CameraSwitchRender::Render() {
		auto& camlist = GetGraphicsEngine().GetCameraManager().GetCameraList();		
		
//#ifndef DW_MASTER
//		if (camlist.size() <= m_cameraNum) {
//			MessageBox(NULL, "カメラの数が足りてません。(画面分割)", "Error", MB_OK);
//			std::abort();
//		}
//#endif

		//メインカメラ変更
		if (camlist.size() <= m_cameraNum) {
			DW_WARNING_MESSAGE(true, "CameraSwitchRender::Render: カメラがありません。初期カメラを使用\n")
			SetMainCamera(nullptr);
		}
		else {
			SetMainCamera(camlist[m_cameraNum]);
		}

		//シェーダに送るポイントライト用カメラ座標更新
		GetGraphicsEngine().GetLightManager().UpdateConstantBuffer();
	}

}