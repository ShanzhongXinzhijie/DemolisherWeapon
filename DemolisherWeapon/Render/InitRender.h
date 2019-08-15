#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class InitRender :
		public IRander
	{
	public:
		InitRender() = default;

		void Render()override;
	};

}