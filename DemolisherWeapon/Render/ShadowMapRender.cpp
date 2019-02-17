#include "DWstdafx.h"
#include "ShadowMapRender.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

ShadowMapRender::ShadowMapRender()
{
}


ShadowMapRender::~ShadowMapRender()
{
	Release();
}

void ShadowMapRender::Init() {
	
	//ライトカメラ行列
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	int stride = sizeof(CMatrix);

	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = static_cast<UINT>(stride * SHADOWMAP_NUM);
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = stride;

	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_lightCameraSB);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVdesc;
	ZeroMemory(&SRVdesc, sizeof(SRVdesc));
	SRVdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	SRVdesc.BufferEx.FirstElement = 0;

	SRVdesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVdesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;

	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_lightCameraSB, &SRVdesc, &m_lightCameraSRV);

	//ブレンドステート
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.IndependentBlendEnable = false;

	blendDesc.RenderTarget[0].BlendEnable = true;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBlendState(&blendDesc, &m_blendState);

	//ビューポート
	m_viewport.Width = (float)ShadowMapResource::MAX_WIDTH;
	m_viewport.Height = (float)ShadowMapResource::MAX_HEIGHT;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
}

void ShadowMapRender::Release() {
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		m_shadowMaps[i].Release();
	}
	
	if (m_lightCameraSB != nullptr) {
		m_lightCameraSB->Release(); m_lightCameraSB = nullptr;
	}
	if (m_lightCameraSRV != nullptr) {
		m_lightCameraSRV->Release(); m_lightCameraSRV = nullptr;
	}

	if (m_blendState) {
		m_blendState->Release(); m_blendState = nullptr;
	}
}

void ShadowMapRender::Render() {

	if (m_setting == enOFF) { return; }

	//もともとの状態を保存
	GameObj::ICamera* oldcam = GetMainCamera();
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);

	//ライトカメラ行列をセット
	{
		CMatrix lightCams[SHADOWMAP_NUM];
		int i2 = 0;
		for (int i = 0; i < SHADOWMAP_NUM; i++) {
			if (!GetShadowMapEnable(i)) { continue; }
			m_shadowMaps[i].GetLightCamera().UpdateMatrix();
			lightCams[i2] = m_shadowMaps[i].GetLightViewProjMatrix();
			i2++;
		}
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_lightCameraSB, 0, NULL, &lightCams, 0, 0
		);
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enRenderZViewProjMatrix, 1, &m_lightCameraSRV);
	}
	int enableShadowNum;
	// RenderTarget設定
	{
		ID3D11RenderTargetView* renderTargetViews[SHADOWMAP_NUM];
		int i2 = 0;
		for (int i = 0; i < SHADOWMAP_NUM; i++) {
			if (!GetShadowMapEnable(i)) { continue; }
			m_shadowMaps[i].ClearShadowMap();//クリア
			renderTargetViews[i2] = m_shadowMaps[i].GetShadowMapRTV();
			i2++;
		}
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(i2, renderTargetViews, NULL);// CShadowMap::GetShadowMapDSV());
		enableShadowNum = i2;
	}

	//シェーダーをZ値書き込み様に
	//個数に応じた
	ModelEffect::SetShaderMode(ModelEffect::enZShader, enableShadowNum-1);

	//ID3D11BlendState* oldBlendState = nullptr;
	//float oldBlendFactor[4] = {};
	//UINT oldBlendSampleMask = 0xFFFFFFFF;
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldBlendFactor, &oldBlendSampleMask);
	
	//ブレンドステートをセット
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(m_blendState, 0, 0xFFFFFFFF);

	//ビューポート設定
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &m_viewport);

	if (enableShadowNum >= 1) {
		//描画
		for (auto& list : m_drawModelList) {
			for (auto& cas : list) {
				cas->Draw(true, enableShadowNum, m_blendState);//裏面・インスタンシング
			}
		}
	}

	/*for (int i = 0; i < SHADOWMAP_NUM; i++) {		

		if (!GetShadowMapEnable(i)) { continue; }

		//描画準備
		m_shadowMaps[i].PreparePreDraw();

		//描画
		for (auto& list : m_drawModelList) {
			for (auto& cas : list) {
				cas->Draw(true);
			}
		}
	}*/

	//ブレンドステートを戻す
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldBlendFactor, oldBlendSampleMask);
	//if (oldBlendState) { oldBlendState->Release(); oldBlendState = nullptr; }

	//シェーダーを通常に
	ModelEffect::SetShaderMode(ModelEffect::enNormalShader);
	
	//カメラ位置戻す
	SetMainCamera(oldcam);

	//ビューポート戻す
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//レンダーターゲット解除
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}
void ShadowMapRender::PostRender() {
	for (auto& list : m_drawModelList) {
		list.clear();
	}
}

}