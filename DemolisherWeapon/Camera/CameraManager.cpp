#include "DWstdafx.h"
#include "CameraManager.h"

namespace DemolisherWeapon {
namespace GameObj {

ICamera::~ICamera() {
	//メインカメラ設定解除
	if (m_isMainCamera) {
		SetMainCamera(nullptr);
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

	CVector4 _screenPos(worldPos.x, worldPos.y, worldPos.z, 1.0f);
	viewProjectionMatrix.Mul(_screenPos);

	CVector3 screenPos;
	screenPos.x = (_screenPos.x / _screenPos.w) * 0.5f + 0.5f;
	screenPos.y = (_screenPos.y / _screenPos.w) *-0.5f + 0.5f;
	screenPos.z = _screenPos.z / _screenPos.w;

	//歪曲収差後の座標を取得
	auto [x, y] = FinalRender::CalcLensDistortion({ screenPos.x , screenPos.y }, this);
	screenPos.x *= screenPos.x/x; screenPos.y *= screenPos.y/y;

	return screenPos;
}
//スクリーン座標からワールド座標を計算する
CVector3 ICamera::CalcWorldPosFromScreenPos( const CVector3& screenPos)  {
	return CalcWorldPosFromScreenPosScreenPos({ screenPos.x * GetGraphicsEngine().GetFrameBuffer_W(), screenPos.y*GetGraphicsEngine().GetFrameBuffer_H(), screenPos.z });
}
CVector3 ICamera::CalcWorldPosFromScreenPosScreenPos(CVector3 screenPos)
{
	//歪曲収差後の座標を取得
	//収差がない状態にする
	screenPos.x /= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y /= GetGraphicsEngine().GetFrameBuffer_H();
	auto[x, y] = FinalRender::CalcLensDistortion({ screenPos.x , screenPos.y }, this);
	screenPos.x = x; screenPos.y = y;
	screenPos.x *= GetGraphicsEngine().GetFrameBuffer_W();
	screenPos.y *= GetGraphicsEngine().GetFrameBuffer_H();

	if (m_change) { UpdateMatrix(); }//必要あれば行列更新

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