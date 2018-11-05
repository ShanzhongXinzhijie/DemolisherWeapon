/*!
 * @brief	�J�v�Z���R���C�_�[�B
 */

#pragma once

#include "ICollider.h"

namespace DemolisherWeapon {

class CapsuleCollider : public ICollider
{
public:
	/*!
	* @brief	�쐬�B
	*@param[in] radius	�J�v�Z���̔��a�B
	*@param[in]	height	�J�v�Z���̍����B
	*/
	void Create(float radius, float height)
	{
		m_shape = std::make_unique<btCapsuleShape>(radius, height);
		m_radius = radius;
		m_height = height;
	}
	btCollisionShape* GetBody() const override
	{
		return m_shape.get();
	}
	float GetRadius() const
	{
		return m_radius;
	}
	float GetHeight() const
	{
		return m_height;
	}
private:
	std::unique_ptr<btCapsuleShape>	m_shape;
	float m_radius = 0.0f;
	float m_height = 0.0f;
};

}