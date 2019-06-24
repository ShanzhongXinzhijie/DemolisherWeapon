/*!
 * @brief	���w�֌W�F�X�B
 */

#pragma once
#include <random>
#include <algorithm>

namespace DemolisherWeapon{

class CMath{
public:
	//�~����
	static constexpr float PI = 3.14159265358979323846f;
	static constexpr float PI2 = PI*2.0f;

	/*!
	 * @brief	degree����radian�ɕϊ��B
	 */
	static constexpr float DegToRad( float deg )
	{
		return deg * (PI/180.0f);
	}
	/*!
	 * @brief	radian����degree�ɕϊ��B
	 */
	static constexpr float RadToDeg( float rad )
	{
		return rad / (PI/180.0f);
	}
	/*!
	 *@brief	t0��t1�Ԃ���`�⊮�B
	 *@details
	 *  ret = t0 + (t1-t0) + rate;
	 *@param[in]	t		��ԗ�	0.0�`1.0
	 */
	static constexpr float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0)*rate;
	}

	//����f�̓����擾
	static constexpr float Square(float f) {
		return f * f;
	}

	//����f��0.0f�`1.0f�ŃN�����v
	static constexpr float Saturate(float f) {
		return Clamp(f,0.0f,1.0f);
	}

	//����f��-1.0f�`1.0f�ŃN�����v
	static constexpr float ClampFromNegOneToPosOne(float f) {
		return Clamp(f, -1.0f, 1.0f);
	}

	//����in��low�`high�̊ԂɎ��߂�
	template<typename Ttype>
	static constexpr Ttype Clamp(Ttype in, Ttype low, Ttype high) {
		return min(max(in, low), high);
	}	

	//����
private:
	static std::random_device rd;
	static std::mt19937 mt;
	static std::uniform_real_distribution<float> zeroToOne;
	static std::uniform_int_distribution<> intRandom;
public:
	//0.0f�`1.0f��float�������擾
	static float RandomZeroToOne() {
		return zeroToOne(mt);
	}
	//0�`RAND_MAX��int�������擾
	static int RandomInt() {
		return intRandom(mt);
	}
};

}