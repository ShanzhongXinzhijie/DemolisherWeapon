#pragma once

namespace DemolisherWeapon {

	class CRayTracingModelRender
	{
	public:
		CRayTracingModelRender() = default;
		CRayTracingModelRender(CModel& model, const CMatrix& worldMat) {
			Init(model, worldMat);
		}
		CRayTracingModelRender(SkinModel& skinmodel) {
			Init(skinmodel);
		}
		CRayTracingModelRender(GameObj::CSkinModelRender& skinmodel) {
			Init(skinmodel);
		}
		CRayTracingModelRender(GameObj::CInstancingModelRender& skinmodel) {
			Init(skinmodel);
		}

		~CRayTracingModelRender() {
			Release();
		}

		void Init(CModel& model, const CMatrix& worldMat);
		void Init(SkinModel& skinmodel);
		void Init(GameObj::CSkinModelRender& model);
		void Init(GameObj::CInstancingModelRender& model);

		void Release();
		
	private:
		CModel* m_model = nullptr;
		const CMatrix* m_worldMat = nullptr;
	};

}