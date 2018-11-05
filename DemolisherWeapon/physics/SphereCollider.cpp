#include "DWstdafx.h"
#include "SphereCollider.h"

namespace DemolisherWeapon {

	/*!
	* @brief	コンストラクタ。
	*/
	SphereCollider::SphereCollider() :
		shape(NULL)
	{
	}
	/*!
	 * @brief	デストラクタ。
	 */
	SphereCollider::~SphereCollider()
	{
		delete shape;
	}
	/*!
	 * @brief	球体コライダーを作成。
	 */
	void SphereCollider::Create(const float radius)
	{
		shape = new btSphereShape(radius);
	}

}