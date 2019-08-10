#include "DWstdafx.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {
	
	void GaussianBlurRender::Init(ID3D11ShaderResourceView*& souce, float dispersion, const CVector2& sampleScale) {

		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();
		
		m_souce = souce;

		//souce�̃e�N�X�`���擾
		ID3D11Resource* pResource = nullptr;
		ID3D11Texture2D* pTex = nullptr;
		souce->GetResource(&pResource);
		pTex = (ID3D11Texture2D*)pResource;
		pResource->Release();
		
		//desc�擾
		D3D11_TEXTURE2D_DESC texDesc;
		pTex->GetDesc(&texDesc);

		//�o�̓e�N�X�`��DESC
		ZeroMemory(&m_texDesc, sizeof(m_texDesc));
		m_texDesc.Width = texDesc.Width;
		m_texDesc.Height = texDesc.Height;
		m_texDesc.MipLevels = 1;
		m_texDesc.ArraySize = 1;
		m_texDesc.Format = texDesc.Format;
		m_texDesc.SampleDesc.Count = 1;
		m_texDesc.SampleDesc.Quality = 0;
		m_texDesc.Usage = D3D11_USAGE_DEFAULT;
		m_texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		m_texDesc.CPUAccessFlags = 0;
		m_texDesc.MiscFlags = 0;

		//�T���v���Ԋu
		m_sampleScale = sampleScale;
		if (m_sampleScale.x < 0.0f) { m_sampleScale.x = (float)m_texDesc.Width; }
		if (m_sampleScale.y < 0.0f) { m_sampleScale.y = (float)m_texDesc.Height; }

		//�o�̓e�N�X�`���̍쐬
		ge.GetD3DDevice()->CreateTexture2D(&m_texDesc, NULL, &m_outputX);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputX, nullptr, &m_outputXRTV);//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputX, nullptr, &m_outputXSRV);//�V�F�[�_�[���\�[�X�r���[

		ge.GetD3DDevice()->CreateTexture2D(&m_texDesc, NULL, &m_outputY);
		ge.GetD3DDevice()->CreateRenderTargetView(m_outputY, nullptr, &m_outputYRTV);//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_outputY, nullptr, &m_outputYSRV);//�V�F�[�_�[���\�[�X�r���[

		//�r���[�|�[�g
		m_viewport.Width = (float)texDesc.Width;
		m_viewport.Height = (float)texDesc.Height;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//�V�F�[�_�[�����[�h�B
		m_vsx.Load("Preset/shader/gaussianblur.fx", "VSXBlur", Shader::EnType::VS);
		m_vsy.Load("Preset/shader/gaussianblur.fx", "VSYBlur", Shader::EnType::VS);
		m_ps.Load("Preset/shader/gaussianblur.fx", "PSBlur", Shader::EnType::PS);

		//�T���v���[
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_samplerState);

		//�萔�o�b�t�@
		ShaderUtil::CreateConstantBuffer(sizeof(SBlurParam), &m_cb);

		//�K�E�X�̏d��
		float total = 0;
		for (int i = 0; i < NUM_WEIGHTS; i++) {
			m_blurParam.weights[i] = expf(-0.5f*(float)(i*i) / dispersion);
			total += 2.0f*m_blurParam.weights[i];
		}
		// �K�i��
		for (int i = 0; i < NUM_WEIGHTS; i++) {
			m_blurParam.weights[i] /= total;
		}
	}
	void GaussianBlurRender::Release() {
		m_outputX->Release();
		m_outputXRTV->Release();
		m_outputXSRV->Release();
		m_outputY->Release();
		m_outputYRTV->Release();
		m_outputYSRV->Release();

		m_cb->Release();
		m_samplerState->Release();
	}

	void GaussianBlurRender::Blur() {

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//�T���v���X�e�[�g��ݒ�B
		rc->PSSetSamplers(0, 1, &m_samplerState);

		//�r���[�|�[�g�L�^
		D3D11_VIEWPORT oldviewport; UINT kaz = 1;
		rc->RSGetViewports(&kaz, &oldviewport);

		//�r���[�|�[�g�ݒ�
		rc->RSSetViewports(1, &m_viewport);

		//Clear
		float clearcolor[4] = {};
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_outputXRTV, clearcolor);
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_outputYRTV, clearcolor);
		
		//XBlur

			//�����_�[�^�[�Q�b�g�̐ݒ�
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_outputXRTV, nullptr);

			//�V�F�[�_�[���\�[�X
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(0, 1, &m_souce);
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_souce);

			//�萔�o�b�t�@
			m_blurParam.offset.x = 16.0f / m_sampleScale.x;
			m_blurParam.offset.y = 0.0f;
			m_blurParam.sampleScale = m_sampleScale.x;
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_blurParam, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);
			rc->VSSetConstantBuffers(0, 1, &m_cb);

			//�V�F�[�_�[��ݒ�
			rc->VSSetShader((ID3D11VertexShader*)m_vsx.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
			//���̓��C�A�E�g��ݒ�B
			rc->IASetInputLayout(m_vsx.GetInputLayout());

			//�`��
			GetEngine().GetGraphicsEngine().DrawFullScreen();

		//YBlur

			//�����_�[�^�[�Q�b�g�̐ݒ�
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_outputYRTV, nullptr);

			//�V�F�[�_�[���\�[�X
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(0, 1, &m_outputXSRV);
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, &m_outputXSRV);

			//�萔�o�b�t�@
			m_blurParam.offset.x = 0.0f;
			m_blurParam.offset.y = 16.0f / m_sampleScale.y;
			m_blurParam.sampleScale = m_sampleScale.y;
			rc->UpdateSubresource(m_cb, 0, nullptr, &m_blurParam, 0, 0);
			rc->PSSetConstantBuffers(0, 1, &m_cb);
			rc->VSSetConstantBuffers(0, 1, &m_cb);

			//�V�F�[�_�[��ݒ�
			rc->VSSetShader((ID3D11VertexShader*)m_vsy.GetBody(), NULL, 0);
			rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
			//���̓��C�A�E�g��ݒ�B
			rc->IASetInputLayout(m_vsy.GetInputLayout());

			//�`��
			GetEngine().GetGraphicsEngine().DrawFullScreen();

		//SRV������
		ID3D11ShaderResourceView* view[] = {
					NULL
		};
		rc->PSSetShaderResources(0, 1, view);

		//�����_�[�^�[�Q�b�g����
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//�r���[�|�[�g�߂�
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);
	}

}