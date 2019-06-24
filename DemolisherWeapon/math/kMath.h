/*!
 * @brief	数学関係色々。
 */

#pragma once
#include <random>
#include <algorithm>

namespace DemolisherWeapon{

class CMath{
public:
	//円周率
	static constexpr float PI = 3.14159265358979323846f;
	static constexpr float PI2 = PI*2.0f;

	/*!
	 * @brief	degreeからradianに変換。
	 */
	static constexpr float DegToRad( float deg )
	{
		return deg * (PI/180.0f);
	}
	/*!
	 * @brief	radianからdegreeに変換。
	 */
	static constexpr float RadToDeg( float rad )
	{
		return rad / (PI/180.0f);
	}
	/*!
	 *@brief	t0とt1間を線形補完。
	 *@details
	 *  ret = t0 + (t1-t0) + rate;
	 *@param[in]	t		補間率	0.0〜1.0
	 */
	static constexpr float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0)*rate;
	}

	//引数fの二乗を取得
	static constexpr float Square(float f) {
		return f * f;
	}

	//引数fを0.0f〜1.0fでクランプ
	static constexpr float Saturate(float f) {
		return Clamp(f,0.0f,1.0f);
	}

	//引数fを-1.0f〜1.0fでクランプ
	static constexpr float ClampFromNegOneToPosOne(float f) {
		return Clamp(f, -1.0f, 1.0f);
	}

	//引数inをlow〜highの間に収める
	template<typename Ttype>
	static constexpr Ttype Clamp(Ttype in, Ttype low, Ttype high) {
		return min(max(in, low), high);
	}	

	//乱数
private:
	static std::random_device rd;
	static std::mt19937 mt;
	static std::uniform_real_distribution<float> zeroToOne;
	static std::uniform_int_distribution<> intRandom;
public:
	//0.0f〜1.0fのfloat乱数を取得
	static float RandomZeroToOne() {
		return zeroToOne(mt);
	}
	//0〜RAND_MAXのint乱数を取得
	static int RandomInt() {
		return intRandom(mt);
	}
};

}