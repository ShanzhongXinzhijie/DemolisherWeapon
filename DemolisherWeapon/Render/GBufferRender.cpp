#include "DWstdafx.h"
#include "GBufferRender.h"

namespace DemolisherWeapon {

GBufferRender::GBufferRender()
{
	
}
GBufferRender::~GBufferRender()
{
	Release();
}

void GBufferRender::Init() {
	GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

	//テクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = (UINT)ge.Get3DFrameBuffer_W();
	texDesc.Height = (UINT)ge.Get3DFrameBuffer_H();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	//アルベド
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_GBufferTex[enGBufferAlbedo]);
	ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferAlbedo], nullptr, &m_GBufferView[enGBufferAlbedo]);//レンダーターゲット
	ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferAlbedo], nullptr, &m_GBufferSRV[enGBufferAlbedo]);//シェーダーリソースビュー

	//ライトパラメーター
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_GBufferTex[enGBufferLightParam]);
	ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferLightParam], nullptr, &m_GBufferView[enGBufferLightParam]);//レンダーターゲット
	ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferLightParam], nullptr, &m_GBufferSRV[enGBufferLightParam]);//シェーダーリソースビュー

	//法線
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_GBufferTex[enGBufferNormal]);
	ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferNormal], nullptr, &m_GBufferView[enGBufferNormal]);//レンダーターゲット
	ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferNormal], nullptr, &m_GBufferSRV[enGBufferNormal]);//シェーダーリソースビュー

	//ビュー座標
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;// DXGI_FORMAT_R16G16B16A16_FLOAT;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_GBufferTex[enGBufferPosition]);
	ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferPosition], nullptr, &m_GBufferView[enGBufferPosition]);//レンダーターゲット
	ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferPosition], nullptr, &m_GBufferSRV[enGBufferPosition]);//シェーダーリソースビュー

	//速度
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;// | D3D11_BIND_UNORDERED_ACCESS;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_GBufferTex[enGBufferVelocity]);
	ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferVelocity], nullptr, &m_GBufferView[enGBufferVelocity]);//レンダーターゲット
	ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferVelocity], nullptr, &m_GBufferSRV[enGBufferVelocity]);//シェーダーリソースビュー
	//速度(ピクセルシェーダ用)
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_GBufferTex[enGBufferVelocityPS]);
	ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferVelocityPS], nullptr, &m_GBufferView[enGBufferVelocityPS]);//レンダーターゲット
	ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferVelocityPS], nullptr, &m_GBufferSRV[enGBufferVelocityPS]);//シェーダーリソースビュー

	//デプスステンシル
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_depthStencilTex);
	
	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	ZeroMemory(&dsv_desc, sizeof(dsv_desc));
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//dsv_desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
	//dsv_desc.Texture2D.MipSlice = 0;
	ge.GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex, &dsv_desc, &m_depthStencilView);//デプスバッファ
	
	//デプス値取得用SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = texDesc.MipLevels;
	ge.GetD3DDevice()->CreateShaderResourceView(m_depthStencilTex, &SRVDesc, &m_depthStencilSRV);
}
void GBufferRender::Release() {
	for (int i = 0; i < enGBufferNum; i++) {
		m_GBufferTex[i]->Release();
		m_GBufferView[i]->Release();
		m_GBufferSRV[i]->Release();
	}
	m_depthStencilTex->Release();
	m_depthStencilView->Release();
	m_depthStencilSRV->Release();
}

void GBufferRender::Render() {
	
#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "カメラが設定されていません!!", "Error", MB_OK);
		std::abort();
	}
#endif
	
	//Gバッファをクリア
	float clearColor[enGBufferNum][4] = {
		{ 0.5f, 0.5f, 0.5f, 0.0f }, //enGBufferAlbedo
		{ 0.0f, 1.0f, 0.0f, 1.0f }, //enGBufferNormal
		{ 0.0f, 0.0f, 0.0f, 0.0f }, //enGBufferPosition
		{ 0.0f, 0.0f, GetMainCamera()->GetFar(), GetMainCamera()->GetFar() }, //enGBufferVelocity
		{ 0.0f, 0.0f, GetMainCamera()->GetFar(), GetMainCamera()->GetFar() }, //enGBufferVelocityPS
		{ 0.0f, 0.0f, 0.0f, 1.0f }, //enGBufferLightParam
	};
	for (int i = 0; i < enGBufferNum; i++) {
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_GBufferView[i], clearColor[i]);
	}
	//デプスステンシルをクリア
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL

	// RenderTarget設定
	ID3D11RenderTargetView* renderTargetViews[enGBufferNum] = { nullptr };
	for (unsigned int i = 0; i < enGBufferNum; i++) {
		renderTargetViews[i] = m_GBufferView[i];
	}
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(enGBufferNum, renderTargetViews, m_depthStencilView);

	//モデル描画
	for (auto& list : m_drawModelList) {
		for (auto& model : list) {
			model->Draw();
		}
	}

	//ラスタライザーステート戻す
	GetEngine().GetGraphicsEngine().ResetRasterizerState();

	//レンダーターゲット解除
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}
void GBufferRender::PostRender() {
	for (auto& list : m_drawModelList) {
		list.clear();
	}
}

}