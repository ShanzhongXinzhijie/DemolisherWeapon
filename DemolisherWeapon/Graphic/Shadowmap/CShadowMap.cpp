#include "DWstdafx.h"
#include "CShadowMap.h"

namespace DemolisherWeapon {

	ShadowMapResource::~ShadowMapResource() {
		Release();
	}

	void ShadowMapResource::Init() {
		if (m_isInit) { return; }

		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//�e�N�X�`���쐬
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = MAX_WIDTH;
		texDesc.Height = MAX_HEIGHT;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = SHADOWMAP_NUM;
		texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		//�V���h�E�}�b�v
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_shadowMapTex);
		ge.GetD3DDevice()->CreateShaderResourceView(m_shadowMapTex, nullptr, &m_shadowMapSRV);//�V�F�[�_�[���\�[�X�r���[

		//�f�v�X�X�e���V��
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.Format = DXGI_FORMAT_D32_FLOAT;
		texDesc.ArraySize = 1;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_depthStencilTex);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ZeroMemory(&dsv_desc, sizeof(dsv_desc));
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		ge.GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex, &dsv_desc, &m_depthStencilView);//�f�v�X�o�b�t�@

		m_isInit = true;
	}

	void ShadowMapResource::Release() {
		if (!m_isInit) { return; }

		m_shadowMapTex->Release();
		m_shadowMapSRV->Release();

		m_depthStencilTex->Release();
		m_depthStencilView->Release();

		m_isInit = false;
	}

	ShadowMapResource CShadowMap::m_resource;
	//bool CShadowMap::m_usedIndexs[ShadowMapResource::SHADOWMAP_NUM] = {};

	CShadowMap::~CShadowMap()
	{
		Release();
	}

	void CShadowMap::Init(UINT width, UINT height, UINT index)
	{
		m_resource.Init();

		Release();

		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		width = min(width, ShadowMapResource::MAX_WIDTH);
		height = min(height, ShadowMapResource::MAX_HEIGHT);

		/*for (int i = 0; i < ShadowMapResource::SHADOWMAP_NUM; i++) {
			if (!m_usedIndexs[i]) {
				m_useIndex = i;
				m_usedIndexs[m_useIndex] = true;
			}
		}*/

		//�����_�[�^�[�Q�b�g
		D3D11_TEXTURE2D_DESC desc;
		m_resource.GetShadowMapTex()->GetDesc(&desc);

		D3D11_RENDER_TARGET_VIEW_DESC drtvd = {
			desc.Format,
			D3D11_RTV_DIMENSION_TEXTURE2DARRAY
		};
		drtvd.Texture2DArray.MipSlice = 0;
		drtvd.Texture2DArray.FirstArraySlice = index;
		drtvd.Texture2DArray.ArraySize = 1;

		ge.GetD3DDevice()->CreateRenderTargetView(m_resource.GetShadowMapTex(), &drtvd, &m_shadowMapRTV);
		
		//�r���[�|�[�g
		m_viewport.Width = (float)width;
		m_viewport.Height = (float)height;
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
		m_lightCam.SetProjMatMode(true);
		m_lightCam.UpdateMatrix();

		m_isInit = true;
	}

	void CShadowMap::Release()
	{
		if (!m_isInit) { return; }

		m_shadowMapRTV->Release();
		
		m_zBias = 0.0f;
		m_cascadeAreaNear = 0.0f, m_cascadeAreaFar = 1.0f;
		m_enable = true;
		m_enablePCSS = true;

		m_isInit = false;
	}

	void CShadowMap::ClearShadowMap() {
		//�N���A
		float clearcolor[4] = {}; clearcolor[0] = 2.0f;
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_shadowMapRTV, clearcolor);
		//�f�v�X�N���A
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_resource.GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void CShadowMap::PreparePreDraw() {
		//�r���[�|�[�g�ݒ�
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &m_viewport);

		ClearShadowMap();

		// RenderTarget�ݒ�
		ID3D11RenderTargetView* renderTargetViews[1] = { m_shadowMapRTV };
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, renderTargetViews, m_resource.GetDSV());

		//�J�����X�V
		m_lightCam.UpdateMatrix();
		//�J�����ʒu�ݒ�
		SetMainCamera(&m_lightCam);
	}
}
