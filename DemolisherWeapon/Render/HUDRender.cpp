#include "DWstdafx.h"
#include "HUDRender.h"

namespace DemolisherWeapon {
	void HUDRender::Init(int HUDNum, const CVector2& screen_min, const CVector2& screen_max, const CVector2& screenSize) {
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		m_HUDNum = HUDNum;

		//�o�̓e�N�X�`��DESC
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = static_cast<UINT>(screenSize.x);
		texDesc.Height = static_cast<UINT>(screenSize.y);
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		//�o�̓e�N�X�`���̍쐬
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_texture.ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_texture.Get(), nullptr, m_RTV.ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_texture.Get(), nullptr, m_SRV.ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[

		//���_�V�F�[�_
		m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		//�s�N�Z���V�F�[�_
		m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

		//�`��}�`	
		m_vertex[0] = {
			{screen_min.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		};
		m_vertex[1] = {
			{screen_max.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		};
		m_vertex[2] = {
			{screen_min.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		};
		m_vertex[3] = {
			{screen_max.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		};
		m_drawSpace.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, m_vertex, 4, m_index);
	}
	void HUDRender::Render() {
		//�r���[�|�[�g�ݒ�
		D3D11_VIEWPORT oldviewport; UINT kaz = 1;
		GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
		//�u�����h�X�e�[�g�ݒ�
		ID3D11BlendState* oldBlendState = nullptr; FLOAT oldf[4]; UINT olduint;
		GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldf, &olduint);

	//HUD�ɕ`��
		//�����_�[�^�[�Q�b�g�̃N���A
		float clearColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
		GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_RTV.Get(), clearColor);
		//�����_�[�^�[�Q�b�g�̐ݒ�
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr);
		//�r���[�|�[�g
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

		//�t���[���o�b�t�@�T�C�Y�̕ύX

		//�`��
		GetEngine().GetGameObjectManager().HUDRender(m_HUDNum);

	//�o�b�N�o�b�t�@��HUD��`��
		//�`�����o�b�N�o�b�t�@�ɂ���
		GetGraphicsEngine().SetBackBufferToRenderTarget();

		//2D�p�̐ݒ�ɂ���
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

		//SRV���Z�b�g
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_SRV.GetAddressOf());

		//�V�F�[�_�[��ݒ�
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
		//���̓��C�A�E�g��ݒ�B
		GetGraphicsEngine().GetD3DDeviceContext()->IASetInputLayout(m_vs.GetInputLayout());
		//�T���v���X�e�[�g��ݒ�B
		auto samp = GetGraphicsEngine().GetCommonStates().LinearClamp();
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetSamplers(0, 1, &samp);
		//�u�����h�X�e�[�g��ݒ�
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().AlphaBlend(), nullptr, 0xFFFFFFFF);

		//�`��
		m_drawSpace.DrawIndexed();

		//SRV������
		ID3D11ShaderResourceView* view[] = { NULL };
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, view);

		//�r���[�|�[�g�߂�
		GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);
		//�u�����h�X�e�[�g�߂�
		if (oldBlendState) {
			GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldf, olduint);
			oldBlendState->Release();
		}
	}
}