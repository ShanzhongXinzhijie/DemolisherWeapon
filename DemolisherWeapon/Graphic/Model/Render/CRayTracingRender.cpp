#include "DWstdafx.h"
#include "CRayTracingRender.h"
#include "Graphic/ReyTracing/ReyTracingEngine.h"

namespace DemolisherWeapon {

	void CRayTracingRender::Init(CModel& model, const CMatrix& worldMat) {
		//レイトレエンジンにモデルとトランスフォーム行列設定
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().RegisterModel(model, &worldMat);
	}
	void CRayTracingRender::Init(SkinModel& model) {
		//レイトレエンジンにモデルとトランスフォーム行列設定
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().RegisterModel(model);
	}
	void CRayTracingRender::Init(GameObj::CSkinModelRender& model) {
		Init(model.GetSkinModel());
	}
	void CRayTracingRender::Init(GameObj::CInstancingModelRender& model) {
		Init(*(model.GetInstancingModel()->GetModelRender().GetSkinModel().GetModel()), model.GetWorldMatrix());
	}

	void CRayTracingRender::Release() {
		//レイトレエンジンから登録解除
	}
}