#include "DWstdafx.h"
#include "CRayTracingRender.h"
#include "Graphic/ReyTracing/ReyTracingEngine.h"

namespace DemolisherWeapon {

	void CRayTracingModelRender::Init(CModel& model, const CMatrix& worldMat) {
		if (GetGraphicsAPI() != enDirectX12) {
			DW_WARNING_MESSAGE(true, "CRayTracingModelRender::Init() DX12�ȊO���Ή�\n")
			return;
		}
		//���C�g���G���W���Ƀ��f���ƃg�����X�t�H�[���s��ݒ�
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().RegisterModel(model, &worldMat);
		m_model = &model;
		m_worldMat = &worldMat;
	}
	void CRayTracingModelRender::Init(SkinModel& model) {
		//���C�g���G���W���Ƀ��f���ƃg�����X�t�H�[���s��ݒ�
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
		//���C�g���G���W������o�^����
		GetGraphicsEngine().GetDX12().GetRayTracingEngine().UnregisterModel(*m_model, m_worldMat);
		m_model = nullptr;
		m_worldMat = nullptr;
	}
}