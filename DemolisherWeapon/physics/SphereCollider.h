#pragma once
#include "ICollider.h"

namespace DemolisherWeapon {

	class SphereCollider :
		public ICollider
	{
	public:
		SphereCollider();
		~SphereCollider();

		/*!
			* @brief	���̃R���C�_�[���쐬�B
		*/
		void Create(const float radius);

		btCollisionShape* GetBody() const override
		{
			return shape;
		}

	private:
		btSphereShape*		shape;
	};

}
