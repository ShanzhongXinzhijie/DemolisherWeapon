#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class CameraSwitchRender :
		public IRander
	{
	public:
		CameraSwitchRender()=default;

		void Init(int cameranum) {
			m_cameraNum = cameranum;
		};

		void Render()override;

	private:
		int m_cameraNum = 0;
	};

}
