#include "DWstdafx.h"
#include "CameraManager.h"

namespace DemolisherWeapon {

namespace GameObj {

ICamera::~ICamera() {
	//ƒƒCƒ“ƒJƒƒ‰İ’è‰ğœ
	if (m_isMainCamera) {
		SetMainCamera(nullptr);
	}
}

PerspectiveCamera::PerspectiveCamera(bool isRegister) : ICamera(isRegister) {
	m_aspect = GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W() / GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
}

OrthoCamera::OrthoCamera(bool isRegister) : ICamera(isRegister) {
	//m_width = GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
	//m_height = GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
}

}

}