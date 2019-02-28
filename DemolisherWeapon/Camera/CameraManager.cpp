#include "DWstdafx.h"
#include "CameraManager.h"

namespace DemolisherWeapon {
namespace GameObj {

ICamera::~ICamera() {
	//���C���J�����ݒ����
	if (m_isMainCamera) {
		SetMainCamera(nullptr);
	}
}

//���[���h���W����X�N���[�����W���v�Z����
CVector3 ICamera::CalcScreenPosFromWorldPosScreenPos(const CVector3& worldPos)  {
	CVector3 screenPos = CalcScreenPosFromWorldPos(worldPos);
	screenPos.x *= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y *= GetGraphicsEngine().GetFrameBuffer_H();
	return screenPos;
}
CVector3 ICamera::CalcScreenPosFromWorldPos(const CVector3& worldPos)
{
	if (m_change) { UpdateMatrix(); }//�K�v����΍s��X�V

	//�r���[�v���W�F�N�V�����s��̍쐬�B
	CMatrix viewProjectionMatrix; viewProjectionMatrix.Mul(m_viewMat, m_projMat);

	CVector4 _screenPos(worldPos.x, worldPos.y, worldPos.z, 1.0f);
	viewProjectionMatrix.Mul(_screenPos);

	CVector3 screenPos;
	screenPos.x = (_screenPos.x / _screenPos.w) * 0.5f + 0.5f;
	screenPos.y = (_screenPos.y / _screenPos.w) *-0.5f + 0.5f;
	screenPos.z = _screenPos.z / _screenPos.w;

	//�c�Ȏ�����̍��W���擾
	auto [x, y] = FinalRender::CalcLensDistortion({ screenPos.x , screenPos.y }, this);
	screenPos.x *= screenPos.x/x; screenPos.y *= screenPos.y/y;

	return screenPos;
}
//�X�N���[�����W���烏�[���h���W���v�Z����
CVector3 ICamera::CalcWorldPosFromScreenPos( const CVector3& screenPos)  {
	return CalcWorldPosFromScreenPosScreenPos({ screenPos.x * GetGraphicsEngine().GetFrameBuffer_W(), screenPos.y*GetGraphicsEngine().GetFrameBuffer_H(), screenPos.z });
}
CVector3 ICamera::CalcWorldPosFromScreenPosScreenPos(CVector3 screenPos)
{
	//�c�Ȏ�����̍��W���擾
	//�������Ȃ���Ԃɂ���
	screenPos.x /= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y /= GetGraphicsEngine().GetFrameBuffer_H();
	screenPos = FinalRender::CalcLensDistortion({ screenPos.x , screenPos.y }, this);
	screenPos.x *= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y *= GetGraphicsEngine().GetFrameBuffer_H();

	if (m_change) { UpdateMatrix(); }//�K�v����΍s��X�V

	CMatrix viewInv = m_viewMat; viewInv.Inverse();
	CMatrix ProjectionInv = m_projMat; ProjectionInv.Inverse();

	CMatrix ViewPortInv;
	ViewPortInv._11 = (float)GetGraphicsEngine().GetFrameBuffer_W() * 0.5f; ViewPortInv._22 = -(float)GetGraphicsEngine().GetFrameBuffer_H() * 0.5f;
	ViewPortInv._41 = (float)GetGraphicsEngine().GetFrameBuffer_W() * 0.5f; ViewPortInv._42 = (float)GetGraphicsEngine().GetFrameBuffer_H() * 0.5f;
	ViewPortInv._33 = 1.0f; ViewPortInv._44 = 1.0f;
	ViewPortInv.Inverse();


	/*CMatrix mBase0;
	mBase0._11 = mBase0._21 = mBase0._31 = mBase0._41 = screenPos.x;
	mBase0._12 = mBase0._22 = mBase0._32 = mBase0._42 = screenPos.y;
	mBase0._13 = mBase0._23 = mBase0._33 = mBase0._43 = z;
	mBase0._14 = mBase0._24 = mBase0._34 = mBase0._44 = 1;*/

	//mBase0.Mul(mBase0, ViewPortInv);
	//mBase0.Mul(mBase0, ProjectionInv);
	//mBase0.Mul(mBase0, viewInv);

	//CVector3 _worldPos;
	////mBase0._14 = 1.0f / mBase0._14;
	//_worldPos.x = mBase0._11 / mBase0._14;
	//_worldPos.y = mBase0._12 / mBase0._14;
	//_worldPos.z = mBase0._13 / mBase0._14;

	//worldPos = _worldPos;


	CVector4 screenPos2; 
	screenPos2.Set(screenPos.x, screenPos.y, screenPos.z, 1.0f);

	ViewPortInv.Mul(screenPos2);
	ProjectionInv.Mul(screenPos2);
	viewInv.Mul(screenPos2);		

	screenPos2 /= screenPos2.w;

	CVector3 worldPos;
	worldPos.x = screenPos2.x;
	worldPos.y = screenPos2.y;
	worldPos.z = screenPos2.z;

	return worldPos;
}

PerspectiveCamera::PerspectiveCamera(bool isRegister) : ICamera(isRegister) {
	m_aspect = GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W() / GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
}

//OrthoCamera::OrthoCamera(bool isRegister) : ICamera(isRegister) {
	//m_width = GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
	//m_height = GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
//}

}
}