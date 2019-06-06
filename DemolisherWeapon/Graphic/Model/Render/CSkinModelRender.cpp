#include "DWstdafx.h"
#include "CSkinModelRender.h"

namespace DemolisherWeapon {
namespace GameObj {

ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCw = nullptr;
ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCCw = nullptr;

CSkinModelRender::CSkinModelRender()
{
	if (!m_mostDepthRSCw) {
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;

		desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);

		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSCw);
	}
	if (!m_mostDepthRSCCw) {
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;

		desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);

		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSCCw);
	}
}
CSkinModelRender::~CSkinModelRender()
{
}

void CSkinModelRender::PreLoopUpdate() {
	if (!m_isInit) { return; }

	//�����W�̍X�V
	m_model.UpdateOldMatrix();
}

void CSkinModelRender::Update() {
	if (!m_isInit || m_animUpdating || !m_isEnableUpdate) { return; }

	if (!m_isUpdated) {
		m_animUpdating = true;
		m_animCon.Update();
		m_animUpdating = false;
	}
	if(!m_isUpdated || !m_isUpdatedWorldMatrix){
		UpdateWorldMatrix(m_isRefreshMode);
	}

	m_isUpdated = true;
}

void CSkinModelRender::PostUpdate() {
	if (!m_isInit) { return; }

	ImNonUpdate();
}

void CSkinModelRender::PostLoopUpdate() {
	if (!m_isInit) { return; }		
	if (!m_isDraw) { return; }

	if (m_isShadowCaster) {
		//�V���h�E�}�b�v�`��O��Ŏ��s���鏈���𑗂�
		if (m_shadowMapPrePost) { GetGraphicsEngine().GetShadowMapRender().AddPrePostAction(m_shadowMapPrePost.get()); }
		//�V���h�E�}�b�v�����_�[�Ƀ��f������
		AddDrawModelToShadowMapRender(&m_model, m_priority, m_isShadowDrawReverse); 
	}
	//3D���f�������_�[�Ƀ��f������
	AddDrawModelToD3Render(&m_model, m_priority);
}

void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips,
	EnFbxUpAxis fbxUpAxis,
	EnFbxCoordinateSystem fbxCoordinate) {

	if (m_isInit) { return; }

	//���f���ǂݍ���
	m_model.Init(filePath, fbxUpAxis, fbxCoordinate);
	
	//�A�j���[�V�����̏������B
	if (animationClips != nullptr) {
		m_animCon.Init(
			m_model,			//�A�j���[�V�����𗬂��X�L�����f���B
			animationClips,		//�A�j���[�V�����N���b�v�̔z��B
			numAnimationClips	//�A�j���[�V�����N���b�v�̐��B
		);
	}

	m_isInit = true;
}

}
}