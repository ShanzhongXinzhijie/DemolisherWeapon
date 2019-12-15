/*!
 * @brief	数学関係色々。
 */

#pragma once
#include <random>
#include <algorithm>

namespace DemolisherWeapon{

class CVector2;
class CVector3;

class CMath{
public:
	//円周率
	static constexpr float PI = 3.14159265358979323846f;
	static constexpr float PI_HALF = PI / 2.0f;
	static constexpr float PI_QUARTER = PI / 4.0f;
	static constexpr float PI2 = PI * 2.0f;

	/*!
	 * @brief	degreeからradianに変換。
	 */
	[[nodiscard]]
	static constexpr float DegToRad( float deg )
	{
		return deg * (PI/180.0f);
	}
	/*!
	 * @brief	radianからdegreeに変換。
	 */
	[[nodiscard]]
	static constexpr float RadToDeg( float rad )
	{
		return rad / (PI/180.0f);
	}
	/*!
	 *@brief	t0とt1間を線形補完。
	 *@details
	 *  ret = t0 + (t1-t0) + rate;
	 *@param[in]	t		補間率	0.0～1.0
	 */
	[[nodiscard]]
	static constexpr float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0)*rate;
	}

	//引数の符号が正なら1.0fを負なら-1.0fを返す
	[[nodiscard]]
	static constexpr float Sign(float f) {
		return f < 0.0f ? -1.0f : 1.0f;
	}

	//引数fの二乗を取得
	[[nodiscard]]
	static constexpr float Square(float f) {
		return f * f;
	}

	//引数fを0.0f～1.0fでクランプ
	[[nodiscard]]
	static constexpr float Saturate(float f) {
		return Clamp(f,0.0f,1.0f);
	}

	//引数fを-1.0f～1.0fでクランプ
	[[nodiscard]]
	static constexpr float ClampFromNegOneToPosOne(float f) {
		return Clamp(f, -1.0f, 1.0f);
	}

	//引数inをlow～highの間に収める
	template<typename Ttype>
	[[nodiscard]]
	static constexpr Ttype Clamp(Ttype in, Ttype low, Ttype high) {
		return min(max(in, low), high);
	}	

	// 線分の衝突(2D)
	//http://marupeke296.com/COL_2D_No10_SegmentAndSegment.html
	static bool ColSegments(
		const CVector2 &seg1Start,	// 線分1の始点
		const CVector2 &seg1Length,	// 線分1の長さベクトル
		const CVector2 &seg2Start,	// 線分2
		const CVector2 &seg2Length, // 線分2の長さベクトル
		CVector2* outPos = nullptr	// 交点（出力）
	);

	//AB CDで構成される２直線の交点(あるいは最近点)を求める
	//http://www.sousakuba.com/Programming/gs_two_lines_intersect.html
/*
	resultはVertex3D ２個の配列

	戻り値
	0 計算できず（平行であったりA=B C=Dのばあい）
	1 交点があった    resultに交点を格納
	2 交点がない　    resultには最近点を格納
*/
	static int IntersectLines(CVector3* result, const CVector3& A, const CVector3& B, const CVector3& C, const CVector3& D);
	
	/// <summary>
	/// 反射ベクトルを計算
	/// </summary>
	/// <param name="front">侵入ベクトル</param>
	/// <param name="normal">法線</param>
	static CVector3 CalcReflectVector(const CVector3& front, const CVector3& normal);
	
	//乱数
private:
	static std::random_device rd;
	static std::mt19937 mt;
	static std::uniform_real_distribution<float> zeroToOne;
	static std::uniform_int_distribution<> intRandom;
public:
	//0.0f～1.0fのfloat乱数を取得
	[[nodiscard]]
	static float RandomZeroToOne() {
		return zeroToOne(mt);
	}
	//0～RAND_MAXのint乱数を取得
	[[nodiscard]]
	static int RandomInt() {
		return intRandom(mt);
	}

	//ブルーノイズを生成
	static void GenerateBlueNoise(int pointNum, const CVector2& min, const CVector2& max, float radius, std::vector<CVector2>& return_points);	
};

}