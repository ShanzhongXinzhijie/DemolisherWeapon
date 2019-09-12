/*!
 * @brief	ベクトルクラス。
 */

#include "DWstdafx.h"
#include "Vector.h"
#include "Matrix.h"

namespace DemolisherWeapon {

//const CVector3 CVector3::Zero = { 0.0f,  0.0f,  0.0f };

CVector2::operator DemolisherWeapon::CVector3()const {
	return CVector3(x, y, 0.0f);
}

/*!
*@brief	行列からクォータニオンを作成。
*/
void CQuaternion::SetRotation(const CMatrix& m)
{
	DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionRotationMatrix(m));
}

void CQuaternion::MakeLookTo(const CVector3& direction, const CVector3& up)
{
	CMatrix mat;
	mat.MakeLookTo(CVector3::Zero(), direction, up);
	SetRotation(mat);
}

void CQuaternion::MakeLookToUseXYAxis(const CVector3& direction) {
	//X軸回転
	CVector3 axisDir = direction*-1.0f; axisDir.x = CVector2(axisDir.x, axisDir.z).Length();
	float XRot = -atan2(axisDir.y, axisDir.x);
	//Y軸回転		
	float YRot = atan2(direction.x*-1.0f, direction.z*-1.0f);
	//クォータニオン作成
	SetRotation(CVector3::AxisX(), XRot);
	Concatenate(CQuaternion(CVector3::AxisY(), YRot));
}

}

