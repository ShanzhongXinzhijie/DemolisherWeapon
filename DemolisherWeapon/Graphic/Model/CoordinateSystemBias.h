#pragma once

namespace DemolisherWeapon {

	/*!
	*@brief	FBXの上方向。
	*/
	enum EnFbxUpAxis {
		enFbxUpAxisY,		//Y-up
		enFbxUpAxisZ,		//Z-up
	};

	//FBXの座標系
	enum EnFbxCoordinateSystem {
		enFbxLeftHanded,		//左手系(directx)
		enFbxRightHanded,		//右手系(3dsmax)
	};

	class CoordinateSystemBias
	{
	public:

		//座標系や軸を変換するバイアスを取得
		static void GetBias(CMatrix& reRotBias, CMatrix& reScrBias, EnFbxUpAxis enFbxUpAxis, EnFbxCoordinateSystem enFbxCoordinate, bool reverse = false) {

			reRotBias = CMatrix::Identity();
			reScrBias = CMatrix::Identity();

			float sign = reverse ? -1.0f : 1.0f;

			//左右反転バイアス
			if (enFbxCoordinate == enFbxRightHanded) {
				reRotBias.MakeRotationY(CMath::PI * sign);
				if (enFbxUpAxis == enFbxUpAxisZ) {
					reScrBias.MakeScaling({ 1.0f,-1.0f,1.0f });
				}
				else {
					reScrBias.MakeScaling({ 1.0f,1.0f,-1.0f });
				}
			}

			//3dsMaxと軸を合わせるためのバイアス。
			if (enFbxUpAxis == enFbxUpAxisZ) {
				//Z-up
				CMatrix mZup;
				mZup.MakeRotationX(CMath::PI * -0.5f * sign);
				reRotBias.Mul(mZup, reRotBias);
			}
		}

	};

}