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

	//もともとの状態を保存
	GameObj::ICamera* oldcam = GetMainCamera();
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);

	//シェーダーをZ値書き込み様に
	ModelEffect::SetShaderMode(ModelEffect::enZShader);	
	
	for (int i = 0; i < SHADOWMAP_NUM; i++) {		

		if (!GetShadowMapEnable(i) || !m_shadowMaps[i].GetIsRenderAndUpdateInterval() || !m_shadowMaps[i].GetIsUpdate()) { continue; }

		//描画準備
		m_shadowMaps[i].PreparePreDraw();

		/*for (auto& list : m_prePostActionList) {
			list->PreModelDraw();
		}*/

		//描画
		for (auto& list : m_drawModelList) {
			for (auto& cas : list) {
				cas.first->Draw(cas.second);
			}
		}

		/*for (auto& list : m_prePostActionList) {
			list->PostModelDraw();
		}*/
	}	

	//シェーダーを通常に
	ModelEffect::SetShaderMode(ModelEffect::enNormalShader);
	
	//カメラ位置戻す
	SetMainCamera(oldcam);

	//ビューポート戻す
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//レンダーターゲット解除
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