#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class InitRender :
		public IRander
	{
	public:
		InitRender() = default;

		void Render()override;

		void Resize()override {};
	};

	class InitRender2D :
		public IRander
	{
	public:
		InitRender2D() = default;

		void Render()override;

		void Resize()override {};
	};

}