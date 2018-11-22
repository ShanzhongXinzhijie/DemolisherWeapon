/*!
 * @brief	�x�N�g���N���X�B
 */

#include "DWstdafx.h"
#include "Vector.h"
#include "Matrix.h"

namespace DemolisherWeapon {

//const CVector3 CVector3::Zero = { 0.0f,  0.0f,  0.0f };

/*!
*@brief	�s�񂩂�N�H�[�^�j�I�����쐬�B
*/
void CQuaternion::SetRotation(const CMatrix& m)
{
	DirectX::XMStoreFloat4(&vec, DirectX::XMQuaternionRotationMatrix(m));
}

}