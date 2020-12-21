#include "DWstdafx.h"
#include "CRayTracingRender.h"
#include "Graphic/ReyTracing/ReyTracingEngine.h"

namespace DemolisherWeapon {

	void CRayTracingModelRender::Init(CModel& model, const CMatrix& worldMat) {
		if (GetGraphicsAPI() != enDirectX12) {
			DW_WARNING_MESSAGE(true, "CRayTracingModelRender::Init() DX12以外未対応\n")
			return;
		}
		//レイトレエンジンにモデルとトランスフォーム行列設定
		m_insStartItr = GetGraphicsEngine().GetDX12().GetRayTracingEngine().RegisterModel(model, &worldMat);
		m_model = &model;
		m_worldMat = &worldMat;
	}
	void CRayTracingModelRender::Init(SkinModel& model) {
		//レイトレエンジンにモデルとトランスフォーム行列設定
		Init(*model.GetModel(),model.GetWorldMatrix());
	}
	void CRayTracingModelRender::Init(GameObj::CSkinModelRender& model) {
		Init(model.GetSkinModel());
	}
	void CRayTracingModelRender::Init(GameObj::CInstancingModelRender& model) {
		Init(*(model.GetInstancingModel()->GetModelRender().GetSkinModel().GetModel()), model.GetWorldMatrix());
	}

	void CRayTracingModelRender::Release() {
		if (m_model == nullptr) {
			return;
		}
		//レイトレエンジンから登録解除
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().UnregisterModel(m_insStartItr, *m_model);
		m_model = nullptr;
		m_worldMat = nullptr;
	}
}