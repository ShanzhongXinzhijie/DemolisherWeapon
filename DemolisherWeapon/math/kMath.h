/*!
 * @brief	”ŠwŠÖŒWFXB
 */

#pragma once
#include <random>
#include <algorithm>

namespace DemolisherWeapon{

class CMath{
public:
	//‰~ü—¦
	static const float PI;
	static const float PI2;
	/*!
	 * @brief	degree‚©‚çradian‚É•ÏŠ·B
	 */
	static float DegToRad( float deg )
	{
		return deg * (PI/180.0f);
	}
	/*!
	 * @brief	radian‚©‚çdegree‚É•ÏŠ·B
	 */
	static float RadToDeg( float rad )
	{
		return rad / (PI/180.0f);
	}
	/*!
	 *@brief	t0‚Æt1ŠÔ‚ğüŒ`•âŠ®B
	 *@details
	 *  ret = t0 + (t1-t0) + rate;
	 *@param[in]	t		•âŠÔ—¦	0.0`1.0
	 */
	static inline float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0)*rate;
	}

	//ˆø”f‚Ì“ñæ‚ğæ“¾
	static inline float Square(float f) {
		return f * f;
	}

	//ˆø”f‚ğ0.0f`1.0f‚ÅƒNƒ‰ƒ“ƒv
	static inline float Saturate(float f) {
		return Clamp(f,0.0f,1.0f);
	}

	//ˆø”f‚ğ-1.0f`1.0f‚ÅƒNƒ‰ƒ“ƒv
	static inline float ClampFromNegOneToPosOne(float f) {
		return Clamp(f, -1.0f, 1.0f);
	}

	//ˆø”in‚ğlow`high‚ÌŠÔ‚Éû‚ß‚é
	template<typename Ttype>
	static inline Ttype Clamp(Ttype in, Ttype low, Ttype high) {
		return min(max(in, low), high);
	}	

	//—”
private:
	static std::random_device rd;
	static std::mt19937 mt;
	static std::uniform_real_distribution<float> zeroToOne;
	static std::uniform_int_distribution<> intRandom;
public:
	//0.0f`1.0f‚Ìfloat—”‚ğæ“¾
	static float RandomZeroToOne() {
		return zeroToOne(mt);
	}
	//0`RAND_MAX‚Ìint—”‚ğæ“¾
	static int RandomInt() {
		return intRandom(mt);
	}
};

}