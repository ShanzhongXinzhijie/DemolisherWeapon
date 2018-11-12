#pragma once

namespace DemolisherWeapon {

	/*!
	*@brief	FBX�̏�����B
	*/
	enum EnFbxUpAxis {
		enFbxUpAxisY,		//Y-up
		enFbxUpAxisZ,		//Z-up
	};

	//FBX�̍��W�n
	enum EnFbxCoordinateSystem {
		enFbxLeftHanded,		//����n(directx)
		enFbxRightHanded,		//�E��n(3dsmax)
	};

	class CoordinateSystemBias
	{
	public:

		//���W�n�⎲��ϊ�����o�C�A�X���擾
		static void GetBias(CMatrix& reRotBias, CMatrix& reScrBias, EnFbxUpAxis enFbxUpAxis, EnFbxCoordinateSystem enFbxCoordinate, bool reverse = false) {

			reRotBias = CMatrix::Identity();
			reScrBias = CMatrix::Identity();

			float sign = reverse ? -1.0f : 1.0f;

			//���E���]�o�C�A�X
			if (enFbxCoordinate == enFbxRightHanded) {
				reRotBias.MakeRotationY(CMath::PI * sign);
				if (enFbxUpAxis == enFbxUpAxisZ) {
					reScrBias.MakeScaling({ 1.0f,-1.0f,1.0f });
				}
				else {
					reScrBias.MakeScaling({ 1.0f,1.0f,-1.0f });
				}
			}

			//3dsMax�Ǝ������킹�邽�߂̃o�C�A�X�B
			if (enFbxUpAxis == enFbxUpAxisZ) {
				//Z-up
				CMatrix mZup;
				mZup.MakeRotationX(CMath::PI * -0.5f * sign);
				reRotBias.Mul(mZup, reRotBias);
			}
		}

	};

}