#include "DWstdafx.h"
#include "CShadowMap.h"

namespace DemolisherWeapon {

	CShadowMap::~CShadowMap()
	{
		Release();
	}

	void CShadowMap::Init(UINT width, UINT height)
	{
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//�e�N�X�`���쐬
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		//�V���h�E�}�b�v
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_shadowMapTex);
		ge.GetD3DDevice()->CreateRenderTargetView(m_shadowMapTex, nullptr, &m_shadowMapView);//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_shadowMapTex, nullptr, &m_shadowMapSRV);//�V�F�[�_�[���\�[�X�r���[
		
		//�f�v�X�X�e���V��
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.Format = DXGI_FORMAT_D32_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_depthStencilTex);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ZeroMemory(&dsv_desc, sizeof(dsv_desc));
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		ge.GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex, &dsv_desc, &m_depthStencilView);//�f�v�X�o�b�t�@

		//�r���[�|�[�g
		m_viewport.Width = (float)texDesc.Width;
		m_viewport.Height = (float)texDesc.Height;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//�J����(�f�t�H���g�ݒ�)
		m_lightCam.SetPos(CVector3(0.0f, 2000.0f, 0.0f));
		m_lightCam.SetTarget(CVector3(0.0f, 0.0f, 0.0f));
		m_lightCam.SetUp(CVector3::AxisZ());
		m_lightCam.SetNear(10.0f);
		m_lightCam.SetFar(4000.0f);
		m_lightCam.SetWidth(2000.0f);
		m_lightCam.SetHeight(2000.0f*(height / width));
		m_lightCam.UpdateMatrix();

		m_isInit = true;
	}

	void CShadowMap::Release()
	{
		if (!m_isInit) { return; }

		m_shadowMapTex->Release();
		m_shadowMapView->Release();
		m_shadowMapSRV->Release();
			
		m_depthStencilTex->Release();
		m_depthStencilView->Release();

		m_isInit = false;
	}

	void CShadowMap::PreparePreDraw() {
		//�r���[�|�[�g�ݒ�
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &m_viewport);

		//�N���A
		float clearcolor[4] = {}; clearcolor[0] = 2.0f;
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_shadowMapView, clearcolor);
		//�f�v�X�N���A
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// RenderTarget�ݒ�
		ID3D11RenderTargetView* renderTargetViews[1] = { m_shadowMapView };
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, renderTargetViews, m_depthStencilView);

		//�J�����X�V
		m_lightCam.UpdateMatrix();
		//�J�����ʒu�ݒ�
		SetMainCamera(&m_lightCam);
	}
}
