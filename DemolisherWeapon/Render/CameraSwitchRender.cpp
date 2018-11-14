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
			MessageBox(NULL, "ƒJƒƒ‰‚Ì”‚ª‘«‚è‚Ä‚Ü‚¹‚ñB(‰æ–Ê•ªŠ„)", "Error", MB_OK);
			std::abort();
		}
#endif

		SetMainCamera(camlist[m_cameraNum]);
	}
}