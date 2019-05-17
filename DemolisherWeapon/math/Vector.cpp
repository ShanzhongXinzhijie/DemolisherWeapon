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

void CQuaternion::MakeLookTo(const CVector3& direction, const CVector3& up)
{
	CMatrix mat;
	mat.MakeLookTo(CVector3::Zero(), direction, up);
	SetRotation(mat);
}

}

