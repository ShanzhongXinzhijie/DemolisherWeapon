/*!
 * @brief	数学関係色々。
 */
#include "DWstdafx.h"
#include "kMath.h"
#include "Vector.h"

namespace DemolisherWeapon {
	
std::random_device CMath::rd;
std::mt19937 CMath::mt(rd());
std::uniform_real_distribution<float> CMath::zeroToOne(0.0f, 1.0f);
std::uniform_int_distribution<> CMath::intRandom(0, RAND_MAX);

bool CMath::ColSegments(
	const CVector2 &seg1Start,	
	const CVector2 &seg1Length,
	const CVector2 &seg2Start,
	const CVector2 &seg2Length,
	CVector2* outPos	
) {

	CVector2 v = seg2Start - seg1Start;
	float Crs_v1_v2 = seg1Length.Cross(seg2Length);
	if (abs(Crs_v1_v2) < FLT_EPSILON) {
		// 平行状態
		return false;
	}

	float Crs_v_v1 = v.Cross(seg1Length);
	float Crs_v_v2 = v.Cross(seg2Length);

	float t1 = Crs_v_v2 / Crs_v1_v2;
	float t2 = Crs_v_v1 / Crs_v1_v2;

	if (t1 + FLT_EPSILON < 0.0f || t1 - FLT_EPSILON > 1.0f || t2 + FLT_EPSILON < 0.0f || t2 - FLT_EPSILON > 1.0f) {
		// 交差していない
		return false;
	}

	if (outPos)*outPos = seg1Start + seg1Length * t1;

	return true;
}

int CMath::IntersectLines(CVector3* result, const CVector3& A, const CVector3& B, const CVector3& C, const CVector3& D)
{
	//A=B C=Dのときは計算できない
	if ((A - B).LengthSq() < FLT_EPSILON || (C - D).LengthSq() < FLT_EPSILON) {
		return 0;
	}

	CVector3 AB = B - A;
	CVector3 CD = D - C;

	CVector3 n1 = AB; n1.Normalize();
	CVector3 n2 = CD; n2.Normalize();

	float work1 = n1.Dot(n2);
	float work2 = 1.0f - work1 * work1;

	//直線が平行な場合は計算できない 平行だとwork2が0になる
	if (abs(work2) < FLT_EPSILON) { return 0; }

	CVector3 AC = C - A;

	float d1 = (AC.Dot(n1) - work1 * AC.Dot(n2)) / work2;
	float d2 = (work1 * AC.Dot(n1) - AC.Dot(n2)) / work2;

	//AB上の最近点
	result[0].x = A.x + d1 * n1.x;
	result[0].y = A.y + d1 * n1.y;
	result[0].z = A.z + d1 * n1.z;

	//BC上の最近点
	result[1].x = C.x + d2 * n2.x;
	result[1].y = C.y + d2 * n2.y;
	result[1].z = C.z + d2 * n2.z;

	//交差の判定 誤差は用途に合わせてください
	if ((result[0] - result[1]).LengthSq() < 1.0f) {
		//交差した
		return 1;
	}

	//交差しなかった。
	return 2;
}

void CMath::GenerateBlueNoise(int pointNum, const CVector2& min, const CVector2& max, float radius, std::vector<CVector2>& return_points) {
	//Dart throwingで生成
	std::unique_ptr<CVector2[]> points = std::make_unique<CVector2[]>(pointNum);
	std::unique_ptr<bool[]> isDead = std::make_unique<bool[]>(pointNum);
	//ランダムな点を生成する
	for (int i = 0; i < pointNum; i++) {
		points[i] = { Lerp(RandomZeroToOne(),min.x,max.x), Lerp(RandomZeroToOne(),min.y,max.y) };
	}
	//点の一定距離内に他の点があったら消滅
	float radiusSq = Square(radius);
	for (int i = 0; i < pointNum; i++) {
		for (int i2 = i+1; i2 < pointNum; i2++) {
			if ((points[i] - points[i2]).LengthSq() < radiusSq) {
				isDead[i] = true;//点死亡
				break;
			}
		}
	}
	//ブルーノイズを返す
	return_points.clear();
	for (int i = 0; i < pointNum; i++) {
		if (!isDead[i]) {//死んでない点
			return_points.emplace_back(points[i]);
		}
	}
}

}