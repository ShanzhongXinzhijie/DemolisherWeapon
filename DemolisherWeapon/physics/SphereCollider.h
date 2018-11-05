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
			* @brief	球体コライダーを作成。
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
