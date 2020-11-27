#pragma once

namespace DemolisherWeapon {

	class CRayTracingRender
	{
	public:
		CRayTracingRender() = default;
		CRayTracingRender(CModel& model, const CMatrix& worldMat) {
			Init(model, worldMat);
		}
		CRayTracingRender(SkinModel& skinmodel) {
			Init(skinmodel);
		}
		CRayTracingRender(GameObj::CSkinModelRender& skinmodel) {
			Init(skinmodel);
		}
		CRayTracingRender(GameObj::CInstancingModelRender& skinmodel) {
			Init(skinmodel);
		}

		~CRayTracingRender() {
			Release();
		}

		void Init(CModel& model, const CMatrix& worldMat);
		void Init(SkinModel& skinmodel);
		void Init(GameObj::CSkinModelRender& model);
		void Init(GameObj::CInstancingModelRender& model);

		void Release();
		
	private:
	};

}