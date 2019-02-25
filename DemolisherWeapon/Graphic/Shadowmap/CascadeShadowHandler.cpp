#include "DWstdafx.h"
#include "CascadeShadowHandler.h"

namespace DemolisherWeapon {

	void CascadeShadowHandler::Init(int cascadeNum, const float splitCoefficient[], const CVector2 resolutions[]) {
		for (int i = 0; i < cascadeNum; i++) {
			m_cShadowMaps.emplace_back(std::make_unique<GameObj::CascadeShadowMap>((UINT)resolutions[i].x, (UINT)resolutions[i].y, splitCoefficient[i], splitCoefficient[i+1]));
		}
	}

	void CascadeShadowHandler::Init(int cascadeNum, const float splitCoefficient[]) {
		CVector2* resolutions = new CVector2[cascadeNum];

		for (int i = 0; i < cascadeNum; i++) {
			resolutions[i] = { 2048, 2048 };
		}

		Init(cascadeNum, splitCoefficient, resolutions);

		delete[] resolutions;
	}

	void CascadeShadowHandler::Init(int cascadeNum){
		float* splitCoefficient = new float[cascadeNum + 1];

		float f = 0.0f;
		for (int i = 0; i < cascadeNum + 1; i++) {
			//splitCoefficient[i] = f;
			//f += 1.0f / cascadeNum;

			if (i == 0) {
				splitCoefficient[i] = 0.0f;
			}
			else {
				splitCoefficient[i] = 1.0f*(pow(4.0f, i - 1) / pow(4.0f, cascadeNum - 1));
			}
		}

		Init(cascadeNum, splitCoefficient);

		delete[] splitCoefficient;
	}
	
}