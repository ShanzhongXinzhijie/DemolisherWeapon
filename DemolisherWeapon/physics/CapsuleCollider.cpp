/*!
 * @brief	カプセルコライダー。
 */

#include "DWstdafx.h"
#include "Physics/CapsuleCollider.h"


namespace DemolisherWeapon {

/*!
	* @brief	デストラクタ。
	*/
CapsuleCollider::~CapsuleCollider()
{
	delete shape;
}

}