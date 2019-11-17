/*!
 * @brief	���w�֌W�F�X�B
 */

#pragma once
#include <random>
#include <algorithm>

namespace DemolisherWeapon{

class CVector2;
class CVector3;

class CMath{
public:
	//�~����
	static constexpr float PI = 3.14159265358979323846f;
	static constexpr float PI_HALF = PI / 2.0f;
	static constexpr float PI_QUARTER = PI / 4.0f;
	static constexpr float PI2 = PI * 2.0f;

	/*!
	 * @brief	degree����radian�ɕϊ��B
	 */
	[[nodiscard]]
	static constexpr float DegToRad( float deg )
	{
		return deg * (PI/180.0f);
	}
	/*!
	 * @brief	radian����degree�ɕϊ��B
	 */
	[[nodiscard]]
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
	[[nodiscard]]
	static constexpr float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0)*rate;
	}

	//�����̕��������Ȃ�1.0f�𕉂Ȃ�-1.0f��Ԃ�
	[[nodiscard]]
	static constexpr float Sign(float f) {
		return f < 0.0f ? -1.0f : 1.0f;
	}

	//����f�̓����擾
	[[nodiscard]]
	static constexpr float Square(float f) {
		return f * f;
	}

	//����f��0.0f�`1.0f�ŃN�����v
	[[nodiscard]]
	static constexpr float Saturate(float f) {
		return Clamp(f,0.0f,1.0f);
	}

	//����f��-1.0f�`1.0f�ŃN�����v
	[[nodiscard]]
	static constexpr float ClampFromNegOneToPosOne(float f) {
		return Clamp(f, -1.0f, 1.0f);
	}

	//����in��low�`high�̊ԂɎ��߂�
	template<typename Ttype>
	[[nodiscard]]
	static constexpr Ttype Clamp(Ttype in, Ttype low, Ttype high) {
		return min(max(in, low), high);
	}	

	// �����̏Փ�(2D)
	//http://marupeke296.com/COL_2D_No10_SegmentAndSegment.html
	static bool ColSegments(
		const CVector2 &seg1Start,	// ����1�̎n�_
		const CVector2 &seg1Length,	// ����1�̒����x�N�g��
		const CVector2 &seg2Start,	// ����2
		const CVector2 &seg2Length, // ����2�̒����x�N�g��
		CVector2* outPos = nullptr	// ��_�i�o�́j
	);

	//AB CD�ō\�������Q�����̌�_(���邢�͍ŋߓ_)�����߂�
	//http://www.sousakuba.com/Programming/gs_two_lines_intersect.html
/*
	result��Vertex3D �Q�̔z��

	�߂�l
	0 �v�Z�ł����i���s�ł�������A=B C=D�̂΂����j
	1 ��_��������    result�Ɍ�_���i�[
	2 ��_���Ȃ��@    result�ɂ͍ŋߓ_���i�[
*/
	static int IntersectLines(CVector3* result, const CVector3& A, const CVector3& B, const CVector3& C, const CVector3& D);
	
	//����
private:
	static std::random_device rd;
	static std::mt19937 mt;
	static std::uniform_real_distribution<float> zeroToOne;
	static std::uniform_int_distribution<> intRandom;
public:
	//0.0f�`1.0f��float�������擾
	[[nodiscard]]
	static float RandomZeroToOne() {
		return zeroToOne(mt);
	}
	//0�`RAND_MAX��int�������擾
	[[nodiscard]]
	static int RandomInt() {
		return intRandom(mt);
	}

	//�u���[�m�C�Y�𐶐�
	static void GenerateBlueNoise(int pointNum, const CVector2& min, const CVector2& max, float radius, std::vector<CVector2>& return_points);	
};

}