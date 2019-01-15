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

	//�f�v�X�X�e���V���X�e�[�g
	/*{
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
	}*/

	//�u���[�V�F�[�_
	m_vsBlur.Load("Preset/shader/shadowblurPS.fx", "VSBlur", Shader::EnType::VS);
	m_psBlurX.Load("Preset/shader/shadowblurPS.fx", "PSXBlur", Shader::EnType::PS);
	m_psBlurY.Load("Preset/shader/shadowblurPS.fx", "PSYBlur", Shader::EnType::PS);
	
	//�T���v���[
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);

	//�u�����h�X�e�[�g
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
	
	//�萔�o�b�t�@
	int bufferSize = sizeof(ShadowBlurConstantBuffer);
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_sbcb);

}

void ShadowMapRender::Release() {

	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		m_shadowMaps[i].Release();
	}

	//m_depthStencilState->Release();
	
	m_samplerState->Release();
	m_sbcb->Release();
}

void ShadowMapRender::Render() {

	//���Ƃ��Ƃ̏�Ԃ�ۑ�
	GameObj::ICamera* oldcam = GetMainCamera();
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;

	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
	//ID3D11DepthStencilState* oldDepthStencilState = nullptr;
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMGetDepthStencilState(&oldDepthStencilState,&kaz);
	
	//�f�v�X�X�e���V���X�e�[�g�ݒ�
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetDepthStencilState(m_depthStencilState, 0);	

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

	//�f�v�X�X�e���V���X�e�[�g�߂�
	//GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetDepthStencilState(oldDepthStencilState, 0);

	//�J�����ʒu�߂�
	SetMainCamera(oldcam);


	//�u���[

	//�V�F�[�_�[��ݒ�
	/*GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShader((ID3D11VertexShader*)m_vsBlur.GetBody(), NULL, 0);
	//���̓��C�A�E�g��ݒ�B
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetInputLayout(m_vsBlur.GetInputLayout());
	//�T���v���X�e�[�g��ݒ�B
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetSamplers(0, 1, &m_samplerState);
	//�u�����h�X�e�[�g��ݒ�B
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(m_blendstate_NonAlpha, blendFactor, 0xffffffff);

	//�R���X�^���g�u�b�t�@
	ShadowBlurConstantBuffer sbCb;
	
	float total = 0;
	for (int i = 0; i < 8; i++) {
		float r = 1.0f + 2.0f * i;
		sbCb.weight[i] = expf(-0.5f*(r*r) / 5.0f);
		total += 2.0f*sbCb.weight[i];
	}
	// �K�i��
	for (int i = 0; i < 8; i++) {
		sbCb.weight[i] /= total;
	}

	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_sbcb, 0, nullptr, &sbCb, 0, 0);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &m_sbcb);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetConstantBuffers(0, 1, &m_sbcb);

	for (int i = 0; i < SHADOWMAP_NUM; i++) {
		for (int i2 = 0; i2 < 2; i2++) {
			//�f�v�X�N���A
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
			//�`��
			GetEngine().GetGraphicsEngine().DrawFullScreen();
		}
	}

	//SRV������
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, view);*/

	//�r���[�|�[�g�߂�
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);

	//�����_�[�^�[�Q�b�g����
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}
void ShadowMapRender::PostRender() {
	m_drawModelList.clear();
}

}