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

//�������6���ʂ��擾
void ICamera::CalcFrustum6Planes(){
	//������̊e����
	CVector3 vZ = GetMainCamera()->GetFront();
	CVector3 vX; vX.Cross(GetMainCamera()->GetUp(), vZ);
	CVector3 vY; vY.Cross(vZ, vX);

	//�ߕ��ʂ̍����ƕ�
	CVector2 nearPlaneHalf;
	GetMainCamera()->GetFrustumPlaneSize(GetMainCamera()->GetNear(), nearPlaneHalf);
	nearPlaneHalf *= 0.5f;

	//�����ʂ̍����ƕ�
	CVector2 farPlaneHalf;
	GetMainCamera()->GetFrustumPlaneSize(GetMainCamera()->GetFar(), farPlaneHalf);
	farPlaneHalf *= 0.5f;

	//�߁E�����ʂ̒��S���W
	CVector3 nearPlaneCenter = GetMainCamera()->GetPos() + vZ * GetMainCamera()->GetNear();
	CVector3 farPlaneCenter = GetMainCamera()->GetPos() + vZ * GetMainCamera()->GetFar();

	//������̊e���ʂ����߂�
	//���@���͓�����
	CVector3 a, b, c;
	for (int i = 0; i < 6; i++) {
		if (i == 0) {//�ߕ���
			a = nearPlaneCenter + vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;//++
			b = nearPlaneCenter + vX * nearPlaneHalf.x - vY * nearPlaneHalf.y;//+-
			c = nearPlaneCenter - vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;//-+
		}
		if (i == 1) {//������
			a = farPlaneCenter - vX * farPlaneHalf.x + vY * farPlaneHalf.y;//-+
			b = farPlaneCenter - vX * farPlaneHalf.x - vY * farPlaneHalf.y;//--
			c = farPlaneCenter + vX * farPlaneHalf.x + vY * farPlaneHalf.y;//++
		}
		if (i == 2) {//�E
			a = farPlaneCenter + vX * farPlaneHalf.x + vY * farPlaneHalf.y;
			b = farPlaneCenter + vX * farPlaneHalf.x - vY * farPlaneHalf.y;
			c = nearPlaneCenter + vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;
		}
		if (i == 3) {//��
			a = farPlaneCenter - vX * farPlaneHalf.x - vY * farPlaneHalf.y;
			b = farPlaneCenter - vX * farPlaneHalf.x + vY * farPlaneHalf.y;
			c = nearPlaneCenter - vX * nearPlaneHalf.x - vY * nearPlaneHalf.y;
		}
		if (i == 4) {//��
			a = farPlaneCenter + vX * farPlaneHalf.x + vY * farPlaneHalf.y;
			b = nearPlaneCenter + vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;
			c = farPlaneCenter - vX * farPlaneHalf.x + vY * farPlaneHalf.y;
		}
		if (i == 5) {//��
			a = farPlaneCenter - vX * farPlaneHalf.x - vY * farPlaneHalf.y;
			b = nearPlaneCenter - vX * nearPlaneHalf.x - vY * nearPlaneHalf.y;
			c = farPlaneCenter + vX * farPlaneHalf.x - vY * farPlaneHalf.y;
		}

		m_planes[i].m_normal.Cross(b - a, c - a); m_planes[i].m_normal *= -1.0f; m_planes[i].m_normal.Normalize();//�@��
		m_planes[i].m_position = a;//���ʏ�̈�_
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
	if (m_change) { UpdateMatrix(); }//�K�v����΍s��X�V

	//�c�Ȏ�����̍��W���擾
	//�������Ȃ���Ԃɂ���
	screenPos.x /= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y /= GetGraphicsEngine().GetFrameBuffer_H();
	screenPos = FinalRender::CalcLensDistortion({ screenPos.x , screenPos.y }, this);
	screenPos.x *= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y *= GetGraphicsEngine().GetFrameBuffer_H();
	
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

void ICamera::UpdateBillboard() {	
	if (!m_needUpdateBillboard) { return; }//�X�V�̕K�v�Ȃ�

	m_needUpdateBillboard = false;

	m_billboardMat = GetViewMatrix();
	m_billboardMat.Inverse();

	//m_billboardQua = CQuaternion::Identity();
	m_billboardQua.SetRotation(m_billboardMat);	
}
CQuaternion ICamera::GetImposterQuaternion(const CVector3& pos) const{
	CQuaternion q;
	q.SetRotation(GetImposterMatrix(pos));
	return q;
}
CMatrix ICamera::GetImposterMatrix(const CVector3& pos) const{
	CMatrix mat;
	mat.MakeLookAt(GetPos(), pos, CVector3::Up());
	mat.Inverse();
	return mat;
}
CQuaternion ICamera::GetBillboardQuaternion() {
	//�K�v����΍s��X�V
	if (m_change) { UpdateMatrix(); }

	UpdateBillboard();
	return m_billboardQua;
}
CMatrix ICamera::GetBillboardMatrix() {
	//�K�v����΍s��X�V
	if (m_change) { UpdateMatrix(); }

	UpdateBillboard();
	return m_billboardMat;
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