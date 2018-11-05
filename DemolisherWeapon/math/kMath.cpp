/*!
 * @brief	数学関係色々。
 */
#include "DWstdafx.h"
#include "kMath.h"


const float CMath::PI = 3.14159265358979323846f;
const float CMath::PI2 = CMath::PI*2.0f;

std::random_device CMath::rd;
std::mt19937 CMath::mt(rd());
std::uniform_real_distribution<float> CMath::zeroToOne(0.0f, 1.0f);
