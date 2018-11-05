/*!
 * @brief	ベクトルクラス。
 */

#include "DWstdafx.h"
#include "Vector.h"
#include "Matrix.h"


//const CVector3 CVector3::Zero = { 0.0f,  0.0f,  0.0f };

/*!
*@brief	行列からクォータニオンを作成。
*/
void CQuaternion::SetRotation(const CMatrix& m)
{
	DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionRotationMatrix(m));
}

