#include "DWstdafx.h"
#include "ShadowMapRender.h"
#include "Graphic/Model/SkinModelEffect.h"

namespace DemolisherWeapon {

ShadowMapRender::ShadowMapRender()
{
}


ShadowMapRender::~ShadowMapRender()
{
	Release();
}

void ShadowMapRender::Init() {
	//GraphicsEngine& ge = GetEngine().GetGraphicsEngine();
}

void ShadowMapRender::Release() {
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		m_shadowMaps[i].Release();
	}
}

void ShadowMapRender::Render() {

	if (m_setting == enOFF) { return; }

	//���Ƃ��Ƃ̏�Ԃ�ۑ�
	GameObj::ICamera* oldcam = GetMainCamera();
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
	
	//�V�F�[�_�[��Z�l�������ݗl��
	ModelEffect::SetShaderMode(ModelEffect::enZShader);

	for (int i = 0; i < SHADOWMAP_NUM; i++) {		

		if (!GetShadowMapEnable(i)) { continue; }

		//�`�揀��
		m_shadowMaps[i].PreparePreDraw();

		//�`��
		for (auto& cas : m_drawModelList) {
			cas->Draw(true);
		}
	}

	//�V�F�[�_�[��ʏ��
	ModelEffect::SetShaderMode(ModelEffect::enNormalShader);
	
	//�J�����ʒu�߂�
	SetMainCamera(oldcam);

	//�r���[�|�[�g�߂�
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//�����_�[�^�[�Q�b�g����
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}
void ShadowMapRender::PostRender() {
	m_drawModelList.clear();
}

}