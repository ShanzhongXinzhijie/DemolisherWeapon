/*!
 * @brief	コライダー。
 */

#pragma once

class btCollisionShape;

namespace DemolisherWeapon {

/*!
 * @brief	コライダーのインターフェースクラス。
 */
class ICollider {
public:
	virtual btCollisionShape* GetBody() const = 0;
};

}