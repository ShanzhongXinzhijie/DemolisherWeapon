/*!
 * @brief	ベクトルクラス。
 */

#include "DWstdafx.h"
#include "Vector.h"
#include "Matrix.h"




/*!
*@brief	行列からクォータニオンを作成。
*/
void CQuaternion::SetRotation(const CMatrix& m)
{
	DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionRotationMatrix(m));
}

