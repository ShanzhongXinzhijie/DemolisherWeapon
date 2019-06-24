/*!
 * @brief	数学関係色々。
 */
#include "DWstdafx.h"
#include "kMath.h"

namespace DemolisherWeapon {
	
std::random_device CMath::rd;
std::mt19937 CMath::mt(rd());
std::uniform_real_distribution<float> CMath::zeroToOne(0.0f, 1.0f);
std::uniform_int_distribution<> CMath::intRandom(0, RAND_MAX);

}