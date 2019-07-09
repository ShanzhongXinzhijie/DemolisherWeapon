#include "DWstdafx.h"
#include "ShadowMapRender.h"
#include "Graphic/Model/SkinModelEffect.h"

namespace DemolisherWeapon {

ShadowMapRender::~ShadowMapRender()
{
	Release();
}

void ShadowMapRender::Release() {
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		m_shadowMaps[i].Release();
	}
}

void ShadowMapRender::Render() {
	if (m_setting == enOFF) { return; }

	/*for (auto& list : m_prePostActionList) {
		list->PreDraw();
	}*/

	//���Ƃ��Ƃ̏�Ԃ�ۑ�
	GameObj::ICamera* oldcam = GetMainCamera();
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);

	//�V�F�[�_�[��Z�l�������ݗl��
	ModelEffect::SetShaderMode(ModelEffect::enZShader);	
	
	for (int i = 0; i < SHADOWMAP_NUM; i++) {		

		if (!GetShadowMapEnable(i) || !m_shadowMaps[i].GetIsRenderAndUpdateInterval() || !m_shadowMaps[i].GetIsUpdate()) { continue; }

		//�`�揀��
		m_shadowMaps[i].PreparePreDraw();

		/*for (auto& list : m_prePostActionList) {
			list->PreModelDraw();
		}*/

		//�`��
		for (auto& list : m_drawModelList) {
			for (auto& cas : list) {
				cas.first->Draw(cas.second);
			}
		}

		/*for (auto& list : m_prePostActionList) {
			list->PostModelDraw();
		}*/
	}	

	//�V�F�[�_�[��ʏ��
	ModelEffect::SetShaderMode(ModelEffect::enNormalShader);
	
	//�J�����ʒu�߂�
	SetMainCamera(oldcam);

	//�r���[�|�[�g�߂�
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//�����_�[�^�[�Q�b�g����
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

	/*for (auto& list : m_prePostActionList) {
		list->PostDraw();
	}*/
}
void ShadowMapRender::PostRender() {
	for (auto& list : m_drawModelList) {
		list.clear();
	}
	//m_prePostActionList.clear();
}

}