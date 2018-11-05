#include "DWstdafx.h"
#include "BoxCollider.h"

namespace DemolisherWeapon {

	/*!
	 * @brief	コンストラクタ。
	 */
	BoxCollider::BoxCollider() :
		shape(NULL)
	{
	}
	/*!
	 * @brief	デストラクタ。
	 */
	BoxCollider::~BoxCollider()
	{
		delete shape;
	}
	/*!
	 * @brief	ボックスコライダーを作成。
	 */
	void BoxCollider::Create(const CVector3& size)
	{
		shape = new btBoxShape(btVector3(size.x*0.5f, size.y*0.5f, size.z*0.5f));
	}
}