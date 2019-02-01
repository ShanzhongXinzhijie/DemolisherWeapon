#include "DWstdafx.h"
#include "FinalRender.h"

namespace DemolisherWeapon {

CFinalRenderTarget::~CFinalRenderTarget() {
	Release();
}
void CFinalRenderTarget::Init() {
	//テクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
	texDesc.Height = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	for (int i = 0; i < 2; i++) {
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_Tex[i]);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateRenderTargetView(m_Tex[i], nullptr, &m_RTV[i]);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_Tex[i], nullptr, &m_SRV[i]);
	}

	//デプスステンシル作成
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_depthStencilTex);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	ZeroMemory(&dsv_desc, sizeof(dsv_desc));
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex, &dsv_desc, &m_depthStencilView);
}
void CFinalRenderTarget::Release() {
	for (int i = 0; i < 2; i++) {
		if (m_Tex[i]) { m_Tex[i]->Release(); m_Tex[i] = nullptr; }
		if (m_RTV[i]) { m_RTV[i]->Release(); m_RTV[i] = nullptr; }
		if (m_SRV[i]) { m_SRV[i]->Release(); m_SRV[i] = nullptr; }
	}
	if (m_depthStencilTex) { m_depthStencilTex->Release(); m_depthStencilTex = nullptr; }
	if (m_depthStencilView) { m_depthStencilView->Release(); m_depthStencilView = nullptr; }
}
void CFinalRenderTarget::Clear(int num) {
	int n = num;
	if (num == -1) { n = m_now; }

	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_RTV[n], ClearColor);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void CFinalRenderTarget::AllClear() {
	for (int i = 0; i < 2; i++) {
		Clear(i);
	}
}
void CFinalRenderTarget::Copy() {
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->CopyResource(m_Tex[m_now^1], m_Tex[m_now]);
}

//ファイナルレンダー
FinalRender::~FinalRender()
{
	Release();
}
void FinalRender::Release() {
	//m_FRT.Release();
	m_samplerState->Release();
}
void FinalRender::Init(const CVector2 screen_min, const CVector2 screen_max) {
	//m_FRT.Init();

	m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
	m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);

	CPrimitive::SVertex vertex[4] = {
		{
			{screen_min.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		},
		{
			{screen_max.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		},
		{
			{screen_min.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		},
		{
			{screen_max.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		},
	};
	int index[4] = { 0,1,2,3 };
	m_drawSpace.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, 4, index);
}
void FinalRender::Render() {
	//描画先をバックバッファにする
	GetEngine().GetGraphicsEngine().SetBackBufferToRenderTarget();

	//ビューポート設定
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
	GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//SRVをセット
	rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());

	//シェーダーを設定
	rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
	rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
	//入力レイアウトを設定。
	rc->IASetInputLayout(m_vs.GetInputLayout());
	//サンプラステートを設定。
	rc->PSSetSamplers(0, 1, &m_samplerState);

	//描画
	m_drawSpace.DrawIndexed();
	//GetEngine().GetGraphicsEngine().DrawFullScreen();

	//SRVを解除
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	rc->PSSetShaderResources(0, 1, view);

	//最終レンダーターゲットをクリア
	GetEngine().GetGraphicsEngine().GetFRT().AllClear();

	//ビューポート戻す
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);
}
//void FinalRender::SetFinalRenderTarget() {
//	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), m_FRT.GetDSV());
//}

}