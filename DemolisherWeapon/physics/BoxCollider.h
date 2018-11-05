#pragma once
#include "ICollider.h"

namespace DemolisherWeapon {
	class BoxCollider :
		public ICollider
	{
	public:
		BoxCollider();
		~BoxCollider();
		
		/*!
		 * @brief	�{�b�N�X�R���C�_�[���쐬�B
		 */
		void Create(const CVector3& size);

		btCollisionShape* GetBody() const override
		{
			return shape;
		}

	private:
		btBoxShape*		shape;
	};
}
