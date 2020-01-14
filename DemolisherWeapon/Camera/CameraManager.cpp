#include "DWstdafx.h"
#include "CameraManager.h"

namespace DemolisherWeapon {
namespace GameObj {

ICamera::~ICamera() {
	//メインカメラ設定解除
	if (m_isMainCamera) {
		SetMainCamera(nullptr);
	}
	//カメラリスト設定解除
	if (m_isCameraListCount > 0) {
		int i = 0;
		for (auto& listcam : ViewCameraList()) {
			if (m_isCameraListCount <= 0) { break; }
			if (listcam == this) {
				SetCameraToList(i, nullptr);
			}
			i++;
		}
	}
}

//視錐台の6平面を取得
void ICamera::CalcFrustum6Planes(){
	//視錐台の各方向
	CVector3 vZ = GetFront();
	CVector3 vX; vX.Cross(GetUp(), vZ); vX.Normalize();//右方向
	CVector3 vY; vY.Cross(vZ, vX); vY.Normalize();//上方向

	//近平面の高さと幅
	CVector2 nearPlaneHalf;
	GetFrustumPlaneSize(GetNear(), nearPlaneHalf);
	nearPlaneHalf *= 0.5f;

	//遠平面の高さと幅
	CVector2 farPlaneHalf;
	GetFrustumPlaneSize(GetFar(), farPlaneHalf);
	farPlaneHalf *= 0.5f;

	//近・遠平面の中心座標
	CVector3 nearPlaneCenter = GetPos() + vZ * GetNear();
	CVector3 farPlaneCenter = GetPos() + vZ * GetFar();

	//視錐台の各平面を求める
	//※法線は内向き
	CVector3 a, b, c;
	for (int i = 0; i < 6; i++) {
		if (i == 0) {//近平面
			a = nearPlaneCenter + vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;//++
			b = nearPlaneCenter + vX * nearPlaneHalf.x - vY * nearPlaneHalf.y;//+-
			c = nearPlaneCenter - vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;//-+
		}
		if (i == 1) {//遠平面
			a = farPlaneCenter - vX * farPlaneHalf.x + vY * farPlaneHalf.y;//-+
			b = farPlaneCenter - vX * farPlaneHalf.x - vY * farPlaneHalf.y;//--
			c = farPlaneCenter + vX * farPlaneHalf.x + vY * farPlaneHalf.y;//++
		}
		if (i == 2) {//右
			a = farPlaneCenter + vX * farPlaneHalf.x + vY * farPlaneHalf.y;
			b = farPlaneCenter + vX * farPlaneHalf.x - vY * farPlaneHalf.y;
			c = nearPlaneCenter + vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;
		}
		if (i == 3) {//左
			a = farPlaneCenter - vX * farPlaneHalf.x - vY * farPlaneHalf.y;
			b = farPlaneCenter - vX * farPlaneHalf.x + vY * farPlaneHalf.y;
			c = nearPlaneCenter - vX * nearPlaneHalf.x - vY * nearPlaneHalf.y;
		}
		if (i == 5) {//上
			a = farPlaneCenter + vX * farPlaneHalf.x + vY * farPlaneHalf.y;
			b = nearPlaneCenter + vX * nearPlaneHalf.x + vY * nearPlaneHalf.y;
			c = farPlaneCenter - vX * farPlaneHalf.x + vY * farPlaneHalf.y;
		}
		if (i == 4) {//下
			a = farPlaneCenter - vX * farPlaneHalf.x - vY * farPlaneHalf.y;
			b = nearPlaneCenter - vX * nearPlaneHalf.x - vY * nearPlaneHalf.y;
			c = farPlaneCenter + vX * farPlaneHalf.x - vY * farPlaneHalf.y;
		}

		m_planes[i].m_normal.Cross(b - a, c - a); m_planes[i].m_normal *= -1.0f; m_planes[i].m_normal.Normalize();//法線
		m_planes[i].m_position = a;//平面上の一点
	}
}

//ワールド座標からスクリーン座標を計算する
CVector3 ICamera::CalcScreenPosFromWorldPosScreenPos(const CVector3& worldPos)  {
	CVector3 screenPos = CalcScreenPosFromWorldPos(worldPos);
	screenPos.x *= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y *= GetGraphicsEngine().GetFrameBuffer_H();
	return screenPos;
}
CVector3 ICamera::CalcScreenPosFromWorldPos(const CVector3& worldPos)
{
	if (m_change) { UpdateMatrix(); }//必要あれば行列更新

	//ビュープロジェクション行列の作成。
	CMatrix viewProjectionMatrix; viewProjectionMatrix.Mul(m_viewMat, m_projMat);

	//ビュープロジェクション行列をワールド座標に適用
	CVector4 _screenPos(worldPos.x, worldPos.y, worldPos.z, 1.0f);
	viewProjectionMatrix.Mul(_screenPos);

	//wで割ってスクリーン座標を求める
	CVector3 screenPos;
	screenPos.x = (_screenPos.x / _screenPos.w) * 0.5f + 0.5f;
	screenPos.y = (_screenPos.y / _screenPos.w) *-0.5f + 0.5f;
	screenPos.z = _screenPos.z / _screenPos.w;

	//歪曲収差後の座標に変換
	//※画面外の座標は変換しない(できない)
	if (screenPos.x >= 0.0f && screenPos.x <= 1.0f || screenPos.y >= 0.0f && screenPos.y <= 1.0f) {
		auto[x, y] = FinalRender::CalcLensDistortion({ screenPos.x , screenPos.y }, this);
		if (screenPos.x >= 0.0f && screenPos.x <= 1.0f) { screenPos.x *= screenPos.x / x; }
		if (screenPos.y >= 0.0f && screenPos.y <= 1.0f) { screenPos.y *= screenPos.y / y; }
	}

	return screenPos;
}
//スクリーン座標からワールド座標を計算する
CVector3 ICamera::CalcWorldPosFromScreenPos( const CVector3& screenPos)  {
	return CalcWorldPosFromScreenPosScreenPos({ screenPos.x * GetGraphicsEngine().GetFrameBuffer_W(), screenPos.y*GetGraphicsEngine().GetFrameBuffer_H(), screenPos.z });
}
CVector3 ICamera::CalcWorldPosFromScreenPosScreenPos(CVector3 screenPos)
{
	if (m_change) { UpdateMatrix(); }//必要あれば行列更新

	//歪曲収差後の座標を取得
	//収差がない状態にする
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
	if (!m_needUpdateBillboard) { return; }//更新の必要なし

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
	//必要あれば行列更新
	if (m_change) { UpdateMatrix(); }

	UpdateBillboard();
	return m_billboardQua;
}
CMatrix ICamera::GetBillboardMatrix() {
	//必要あれば行列更新
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