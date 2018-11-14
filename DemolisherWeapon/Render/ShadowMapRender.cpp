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
	GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

	//テクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = (UINT)ge.Get3DFrameBuffer_W()*2;
	texDesc.Height = (UINT)ge.Get3DFrameBuffer_H()*2;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	//シャドウマップ
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		for (int i2 = 0; i2 < 2; i2++) {
			ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_shadowMapTex[i][i2]);
			ge.GetD3DDevice()->CreateRenderTargetView(m_shadowMapTex[i][i2], nullptr, &m_shadowMapView[i][i2]);//レンダーターゲット
			ge.GetD3DDevice()->CreateShaderResourceView(m_shadowMapTex[i][i2], nullptr, &m_shadowMapSRV[i][i2]);//シェーダーリソースビュー
		}
	}

	//デプスステンシル
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_depthStencilTex);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	ZeroMemory(&dsv_desc, sizeof(dsv_desc));
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	ge.GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex, &dsv_desc, &m_depthStencilView);//デプスバッファ

	//ビューポート
	m_viewport.Width = (float)texDesc.Width;
	m_viewport.Height = (float)texDesc.Height;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	//デプスステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_GREATER;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ge.GetD3DDevice()->CreateDepthStencilState(&desc, &m_depthStencilState);
	}
	//ラスタライザーステート(背面描画)
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		ge.GetD3DDevice()->CreateRasterizerState(&desc, &m_rasterizerState);
	}

	//ブラーシェーダ
	m_vsBlur.Load("Preset/shader/shadowblurPS.fx", "VSBlur", Shader::EnType::VS);
	m_psBlurX.Load("Preset/shader/shadowblurPS.fx", "PSXBlur", Shader::EnType::PS);
	m_psBlurY.Load("Preset/shader/shadowblurPS.fx", "PSYBlur", Shader::EnType::PS);
	
	//サンプラー
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);

	//ブレンドステート
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_GREEN;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendEnable = false;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBlendState(&blendDesc, &m_blendstate_NonAlpha);
	
	//定数バッファ
	int bufferSize = sizeof(ShadowBlurConstantBuffer);
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_sbcb);

	//ライト
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		CVector3 pos(-0.5f, 0.5f, 1.0f); pos.Normalize();
		m_lightCam[i].SetPos(pos*2000.0f);
		m_lightCam[i].SetTarget(CVector3(0.0f, 0.0f, 1.0f));
		m_lightCam[i].SetUp(CVector3::Up());
		m_lightCam[i].SetNear(10.0f);
		m_lightCam[i].SetFar(4000.0f);
		m_lightCam[i].SetWidth(2000.0f);
		m_lightCam[i].SetHeight(2000.0f*(9.0f/16.0f));
		m_lightCam[i].UpdateMatrix();
	}
}

void ShadowMapRender::Release() {
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		for (int i2 = 0; i2 < 2; i2++) {
			m_shadowMapTex[i][i2]->Release();
			m_shadowMapView[i][i2]->Release();
			m_shadowMapSRV[i][i2]->Release();
		}
	}
	m_depthStencilTex->Release();
	m_depthStencilView->Release();

	m_depthStencilState->Release();
	m_rasterizerState->Release();

	m_samplerState->Release();
	m_sbcb->Release();
}

void ShadowMapRender::Render() {

	GameObj::ICamera* oldcam = GetMainCamera();
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;

	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
	//ID3D11DepthStencilState* oldDepthStencilState = nullptr;
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMGetDepthStencilState(&oldDepthStencilState,&kaz);
	//ID3D11RasterizerState* oldRasterizerState = nullptr;
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetState(&oldRasterizerState);

	//ビューポート設定
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &m_viewport);

	//デプスステンシルステート設定
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetDepthStencilState(m_depthStencilState, 0);

	//ラスタライザーステート設定
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetState(m_rasterizerState);

	//クリア
	float clearcolor[SHADOWMAP_NUM][4] = {};
	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		clearcolor[i][0] = 2.0f;
		for (int i2 = 0; i2 < 2; i2++) {
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_shadowMapView[i][i2], clearcolor[i]);
		}
	}

	//シェーダーをZ値書き込み様に
	ModelEffect::SetShaderMode(ModelEffect::enZShader);

	for (int i = 0; i < SHADOWMAP_NUM; i++) {		
		//デプスクリア
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		
		// RenderTarget設定
		ID3D11RenderTargetView* renderTargetViews[1] = { m_shadowMapView[i][0] };
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, renderTargetViews, m_depthStencilView);

		//カメラ位置設定
		SetMainCamera(&m_lightCam[i]);

		//描画
		for (auto& cas : m_drawModelList) {
			cas->Draw();// true);
		}
	}

	//シェーダーを通常に
	ModelEffect::SetShaderMode(ModelEffect::enNormalShader);

	//ラスタライザーステート戻す
	GetEngine().GetGraphicsEngine().ResetRasterizerState();
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetState(oldRasterizerState);

	//デプスステンシルステート戻す
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetDepthStencilState(oldDepthStencilState, 0);

	//ビューポート戻す
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//カメラ位置戻す
	SetMainCamera(oldcam);


	//ブラー

	//シェーダーを設定
	/*GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShader((ID3D11VertexShader*)m_vsBlur.GetBody(), NULL, 0);
	//入力レイアウトを設定。
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetInputLayout(m_vsBlur.GetInputLayout());
	//サンプラステートを設定。
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetSamplers(0, 1, &m_samplerState);
	//ブレンドステートを設定。
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(m_blendstate_NonAlpha, blendFactor, 0xffffffff);

	//コンスタントブッファ
	ShadowBlurConstantBuffer sbCb;
	
	float total = 0;
	for (int i = 0; i < 8; i++) {
		float r = 1.0f + 2.0f * i;
		sbCb.weight[i] = expf(-0.5f*(r*r) / 5.0f);
		total += 2.0f*sbCb.weight[i];
	}
	// 規格化
	for (int i = 0; i < 8; i++) {
		sbCb.weight[i] /= total;
	}

	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_sbcb, 0, nullptr, &sbCb, 0, 0);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &m_sbcb);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetConstantBuffers(0, 1, &m_sbcb);

	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		for (int i2 = 0; i2 < 2; i2++) {
			//デプスクリア
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

			if (i2 == 0) {
				ID3D11RenderTargetView* renderTargetViews[1] = { m_shadowMapView[i][1] };
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, renderTargetViews, m_depthStencilView);
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_shadowMapSRV[i][0]);
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShader((ID3D11PixelShader*)m_psBlurX.GetBody(), NULL, 0);
			}
			else {
				ID3D11RenderTargetView* renderTargetViews[1] = { m_shadowMapView[i][0] };
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, renderTargetViews, m_depthStencilView);
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_shadowMapSRV[i][1]);
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShader((ID3D11PixelShader*)m_psBlurY.GetBody(), NULL, 0);
			}
			//描画
			GetEngine().GetGraphicsEngine().DrawFullScreen();
		}
	}

	//SRVを解除
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, view);*/

	//ビューポート戻す
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//レンダーターゲット解除
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}
void ShadowMapRender::PostRender() {
	m_drawModelList.clear();
}

}