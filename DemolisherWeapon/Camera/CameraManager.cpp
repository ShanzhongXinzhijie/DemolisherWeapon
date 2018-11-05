#include "DWstdafx.h"
#include "CameraManager.h"

namespace DemolisherWeapon {

namespace GameObj {

PerspectiveCamera::PerspectiveCamera(bool isRegister) : ICamera(isRegister) {
	m_aspect = GetEngine().GetGraphicsEngine().GetFrameBuffer_W() / GetEngine().GetGraphicsEngine().GetFrameBuffer_H();
}

OrthoCamera::OrthoCamera(bool isRegister) : ICamera(isRegister) {
	//m_width = GetEngine().GetGraphicsEngine().GetFrameBuffer_W();
	//m_height = GetEngine().GetGraphicsEngine().GetFrameBuffer_H();
}

}

CameraManager::CameraManager()
{
}
CameraManager::~CameraManager()
{
}

}