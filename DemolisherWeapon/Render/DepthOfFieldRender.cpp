#include "DWstdafx.h"
#include "DepthOfFieldRender.h"

namespace DemolisherWeapon {

	DepthOfFieldRender::DepthOfFieldRender()
	{
	}
	DepthOfFieldRender::~DepthOfFieldRender()
	{
		Release();
	}

	void DepthOfFieldRender::Init(float texScale) {
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//�����p�T���v���[
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		ge.GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);

		//�k���e�N�X�`���T�C�Y�Z�o
		m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W() * texScale), m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H() * texScale);

		//�k���e�N�X�`���쐬
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = m_textureSizeX;
		texDesc.Height = m_textureSizeY;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_tex);
		ge.GetD3DDevice()->CreateShaderResourceView(m_tex, nullptr, &m_SRV);//�V�F�[�_�[���\�[�X�r���[
		ge.GetD3DDevice()->CreateRenderTargetView(m_tex, nullptr, &m_RTV);//�����_�[�^�[�Q�b�g

		//�r���[�|�[�g
		m_viewport.Width = (float)texDesc.Width;
		m_viewport.Height = (float)texDesc.Height;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//�k���`��p�V�F�[�_
		m_vsResize.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		m_psResize.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

		//�K�E�X�u���[
		//m_gaussBlur.Init(m_SRV, 0.5f);

		//�����V�F�[�_
		m_vsCompo.Load("Preset/shader/DOF.fx", "VSMain", Shader::EnType::VS);
		m_psCompo.Load("Preset/shader/DOF.fx", "PSMain", Shader::EnType::PS);
		//�萔�o�b�t�@
		ShaderUtil::CreateConstantBuffer(sizeof(SPSConstantBuffer), &m_cb);
	}
	void DepthOfFieldRender::Release() {
		if (m_tex) { m_tex->Release(); m_tex = nullptr; }
		if (m_SRV) { m_SRV->Release(); m_SRV = nullptr; }
		if (m_RTV) { m_RTV->Release(); m_RTV = nullptr; }
		if (m_cb) { m_cb->Release(); m_cb = nullptr; }
		if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
	}

	void DepthOfFieldRender::Render() {
		if (!m_enable) { return; }

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//�k��
		{
			//�`�����N���A
			float clearColor[4] = {
				0.0f, 0.0f, 0.0f, 0.0f
			};
			rc->ClearRenderTargetView(m_RTV, clearColor);

			//�r���[�|�[�g�L�^
			D3D11_VIEWPORT oldviewport; UINT kaz = 1;
			rc->RSGetViewports(&kaz, &oldviewport);
			//�r���[�|�[�g�ݒ�
			rc->RSSetViewports(1, &m_viewport);

			//SRV���Z�b�g	
			rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());

			//�`�����k���e�N�X�`���ɂ���
			rc->OMSetRenderTargets(1, &m_RTV, nullptr);

			//�V�F�[�_�[��ݒ�
			rc->VSSetShader((ID3D11VertexShader*)m_vsResize.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_psResize.GetBody(), NULL, 0);
			//���̓��C�A�E�g��ݒ�B
			rc->IASetInputLayout(m_vsResize.GetInputLayout());
			//�T���v���X�e�[�g��ݒ�B
			rc->PSSetSamplers(0, 1, &m_samplerState);

			//�`��
			GetEngine().GetGraphicsEngine().DrawFullScreen();

			//SRV������
			ID3D11ShaderResourceView* view[] = {
						NULL
			};
			rc->PSSetShaderResources(0, 1, view);

			//�����_�[�^�[�Q�b�g����
			rc->OMSetRenderTargets(0, NULL, NULL);

			//�r���[�|�[�g�߂�
			rc->RSSetViewports(1, &oldviewport);
		}

		//�ڂ���
		//m_gaussBlur.Blur();

		//����
		{
			//SRV���Z�b�g	
			rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
			rc->PSSetShaderResources(1, 1, &m_SRV);// m_gaussBlur.GetSRV());
			rc->PSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));

			//�ŏI�����_�[�^�[�Q�b�g�̃X���b�v
			GetEngine().GetGraphicsEngine().GetFRT().Swap();

			//�`�����ŏI�����_�[�^�[�Q�b�g�ɂ���
			GetEngine().GetGraphicsEngine().SetFinalRenderTarget();

			//�V�F�[�_�[��ݒ�
			rc->VSSetShader((ID3D11VertexShader*)m_vsCompo.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_psCompo.GetBody(), NULL, 0);
			//���̓��C�A�E�g��ݒ�B
			rc->IASetInputLayout(m_vsCompo.GetInputLayout());
			//�T���v���X�e�[�g��ݒ�B
			rc->PSSetSamplers(0, 1, &m_samplerState);

			//�萔�o�b�t�@
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_cbSetting, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);

			//�`��
			GetEngine().GetGraphicsEngine().DrawFullScreen();

			//SRV������
			ID3D11ShaderResourceView* view[] = {
						NULL
			};
			rc->PSSetShaderResources(0, 1, view);
			rc->PSSetShaderResources(1, 1, view);
			rc->PSSetShaderResources(2, 1, view);

			//�����_�[�^�[�Q�b�g����
			rc->OMSetRenderTargets(0, NULL, NULL);
		}
	}

}