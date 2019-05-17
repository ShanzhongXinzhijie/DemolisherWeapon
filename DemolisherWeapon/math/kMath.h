/*!
 * @brief	���w�֌W�F�X�B
 */

#pragma once
#include <random>

namespace DemolisherWeapon{

class CMath{
public:
	static const float PI;
	static const float PI2;
	/*!
	 * @brief	degree����radian�ɕϊ��B
	 */
	static float DegToRad( float deg )
	{
		return deg * (PI/180.0f);
	}
	/*!
	 * @brief	radian����degree�ɕϊ��B
	 */
	static float RadToDeg( float rad )
	{
		return rad / (PI/180.0f);
	}
	/*!
	 *@brief	t0��t1�Ԃ���`�⊮�B
	 *@details
	 *  ret = t0 + (t1-t0) + rate;
	 *@param[in]	t		��ԗ�	0.0�`1.0
	 */
	static inline float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0)*rate;
	}

	template<typename Ttype>
	static inline Ttype Clamp(Ttype in, Ttype low, Ttype high) {
		return min(max(in, low), high);
	}	

	//����
private:
	static std::random_device rd;
	static std::mt19937 mt;
	static std::uniform_real_distribution<float> zeroToOne;
	static std::uniform_int_distribution<> intRandom;
public:
	static float RandomZeroToOne() {
		return zeroToOne(mt);
	}
	static int RandomInt() {
		return intRandom(mt);
	}
};

}