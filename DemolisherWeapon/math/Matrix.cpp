/*!
 * @brief	行列。
 */

#include "DWstdafx.h"
#include "Matrix.h"

namespace DemolisherWeapon {

	//namespace {
	//	bool transformRotMatToQuaternion(
	//		float &qx, float &qy, float &qz, float &qw,
	//		float m11, float m12, float m13,
	//		float m21, float m22, float m23,
	//		float m31, float m32, float m33
	//	) {
	//		// 最大成分を検索
	//		float elem[4]; // 0:x, 1:y, 2:z, 3:w
	//		elem[0] = m11 - m22 - m33 + 1.0f;
	//		elem[1] = -m11 + m22 - m33 + 1.0f;
	//		elem[2] = -m11 - m22 + m33 + 1.0f;
	//		elem[3] = m11 + m22 + m33 + 1.0f;

	//		unsigned biggestIndex = 0;
	//		for (int i = 1; i < 4; i++) {
	//			if (elem[i] > elem[biggestIndex])
	//				biggestIndex = i;
	//		}

	//		if (elem[biggestIndex] < 0.0f)
	//			return false; // 引数の行列に間違いあり！

	//		// 最大要素の値を算出
	//		float *q[4] = { &qx, &qy, &qz, &qw };
	//		float v = sqrtf(elem[biggestIndex]) * 0.5f;
	//		*q[biggestIndex] = v;
	//		float mult = 0.25f / v;

	//		switch (biggestIndex) {
	//		case 0: // x
	//			*q[1] = (m12 + m21) * mult;
	//			*q[2] = (m31 + m13) * mult;
	//			*q[3] = (m23 - m32) * mult;
	//			break;
	//		case 1: // y
	//			*q[0] = (m12 + m21) * mult;
	//			*q[2] = (m23 + m32) * mult;
	//			*q[3] = (m31 - m13) * mult;
	//			break;
	//		case 2: // z
	//			*q[0] = (m31 + m13) * mult;
	//			*q[1] = (m23 + m32) * mult;
	//			*q[3] = (m12 - m21) * mult;
	//			break;
	//		case 3: // w
	//			*q[0] = (m23 - m32) * mult;
	//			*q[1] = (m31 - m13) * mult;
	//			*q[2] = (m12 - m21) * mult;
	//			break;
	//		}

	//		return true;
	//	}
	//}

	void CMatrix::Interpolate(CMatrix m1, CMatrix m2, float blendTrans, float blendRot, float blendScale) {
		//平行移動の補完
		CVector3 move;
		move.Lerp(
			blendTrans,
			*(CVector3*)m1.m[3],
			*(CVector3*)m2.m[3]
		);
		//平行移動成分を削除。
		m1.m[3][0] = 0.0f;
		m1.m[3][1] = 0.0f;
		m1.m[3][2] = 0.0f;
		m2.m[3][0] = 0.0f;
		m2.m[3][1] = 0.0f;
		m2.m[3][2] = 0.0f;

		//拡大成分の補間。
		CVector3 vBoneScale, vBoneScalePrev;
		vBoneScale.x = (*(CVector3*)m2.m[0]).Length();
		vBoneScale.y = (*(CVector3*)m2.m[1]).Length();
		vBoneScale.z = (*(CVector3*)m2.m[2]).Length();
		vBoneScalePrev.x = (*(CVector3*)m1.m[0]).Length();
		vBoneScalePrev.y = (*(CVector3*)m1.m[1]).Length();
		vBoneScalePrev.z = (*(CVector3*)m1.m[2]).Length();
		CVector3 scale;
		scale.Lerp(
			blendScale,
			vBoneScalePrev,
			vBoneScale
		);
		//拡大成分を除去。
		m2.m[0][0] /= vBoneScale.x;
		m2.m[0][1] /= vBoneScale.x;
		m2.m[0][2] /= vBoneScale.x;
		m2.m[1][0] /= vBoneScale.y;
		m2.m[1][1] /= vBoneScale.y;
		m2.m[1][2] /= vBoneScale.y;
		m2.m[2][0] /= vBoneScale.z;
		m2.m[2][1] /= vBoneScale.z;
		m2.m[2][2] /= vBoneScale.z;
		m1.m[0][0] /= vBoneScalePrev.x;
		m1.m[0][1] /= vBoneScalePrev.x;
		m1.m[0][2] /= vBoneScalePrev.x;
		m1.m[1][0] /= vBoneScalePrev.y;
		m1.m[1][1] /= vBoneScalePrev.y;
		m1.m[1][2] /= vBoneScalePrev.y;
		m1.m[2][0] /= vBoneScalePrev.z;
		m1.m[2][1] /= vBoneScalePrev.z;
		m1.m[2][2] /= vBoneScalePrev.z;

		//回転の補完
		CQuaternion qBone, qBonePrev;
		/*transformRotMatToQuaternion(qBone.x, qBone.y, qBone.z, qBone.w,
			m2.m[0][0], m2.m[0][1], m2.m[0][2],
			m2.m[1][0], m2.m[1][1], m2.m[1][2],
			m2.m[2][0], m2.m[2][1], m2.m[2][2]
			);*/
		qBone.SetRotation(m2);
		/*transformRotMatToQuaternion(qBonePrev.x, qBonePrev.y, qBonePrev.z, qBonePrev.w,
			m1.m[0][0], m1.m[0][1], m1.m[0][2],
			m1.m[1][0], m1.m[1][1], m1.m[1][2],
			m1.m[2][0], m1.m[2][1], m1.m[2][2]
		);*/
		qBonePrev.SetRotation(m1);
		CQuaternion rot;
		rot.Slerp(blendRot, qBonePrev, qBone);

		//拡大行列を作成。
		CMatrix scaleMatrix;
		scaleMatrix.MakeScaling(scale);
		//回転行列を作成。
		CMatrix rotMatrix;
		rotMatrix.MakeRotationFromQuaternion(rot);
		//平行移動行列を作成。
		CMatrix transMat;
		transMat.MakeTranslation(move);
		//全部を合成して、ボーン行列を作成。
		m1.Mul(scaleMatrix, rotMatrix);
		m1.Mul(m1, transMat);

		mat = m1.mat;
	}

}