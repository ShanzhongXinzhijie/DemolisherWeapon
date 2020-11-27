#include "DWstdafx.h"
#include "CRayTracingRender.h"
#include "Graphic/ReyTracing/ReyTracingEngine.h"

namespace DemolisherWeapon {

	void CRayTracingRender::Init(CModel& model, const CMatrix& worldMat) {
		//���C�g���G���W���Ƀ��f���ƃg�����X�t�H�[���s��ݒ�
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().RegisterModel(model, &worldMat);
	}
	void CRayTracingRender::Init(SkinModel& model) {
		//���C�g���G���W���Ƀ��f���ƃg�����X�t�H�[���s��ݒ�
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().RegisterModel(model);
	}
	void CRayTracingRender::Init(GameObj::CSkinModelRender& model) {
		Init(model.GetSkinModel());
	}
	void CRayTracingRender::Init(GameObj::CInstancingModelRender& model) {
		Init(*(model.GetInstancingModel()->GetModelRender().GetSkinModel().GetModel()), model.GetWorldMatrix());
	}

	void CRayTracingRender::Release() {
		//���C�g���G���W������o�^����
	}
}