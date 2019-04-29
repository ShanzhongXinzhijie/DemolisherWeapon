#include "DWstdafx.h"
#include "BloomRender.h"

namespace DemolisherWeapon {
	BloomRender::BloomRender()
	{
	}
	BloomRender::~BloomRender()
	{
		Release();
	}

	void BloomRender::Init(float texScale)
	{
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//�R���s���[�g�V�F�[�_
		m_cs.Load("Preset/shader/Bloom.fx", "CSmain", Shader::EnType::CS);

		//�e�N�X�`���T�C�Y�Z�o
		m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W() * texScale), m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H() * texScale);

		//�o�̓e�N�X�`���쐬
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
		texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_tex);
		ge.GetD3DDevice()->CreateShaderResourceView(m_tex, nullptr, &m_SRV);//�V�F�[�_�[���\�[�X�r���[
		ge.GetD3DDevice()->CreateRenderTargetView(m_tex, nullptr, &m_RTV);//�����_�[�^�[�Q�b�g

		//OutPutUAV
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC	uavDesc;
			ZeroMemory(&uavDesc, sizeof(uavDesc));

			uavDesc.Format = texDesc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			HRESULT	hr;
			hr = ge.GetD3DDevice()->CreateUnorderedAccessView(m_tex, &uavDesc, &m_outputUAV);
		}

		//�萔�o�b�t�@
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = (((sizeof(SCSConstantBuffer) - 1) / 16) + 1) * 16;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		ge.GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cb);

		//���ȃe�N�X�`��
		HRESULT hr = DirectX::CreateDDSTextureFromFile(ge.GetD3DDevice(), L"Preset/sprite/Blackbody_Enumerated_6500K.dds", nullptr, &m_interferenceFringesSRV);
		
		//�K�E�X�u���[
		m_gaussBlur.Init(m_SRV, 1.5f);

		//���Z�u�����h�X�e�[�g
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
		ge.GetD3DDevice()->CreateBlendState(&blendDesc, &m_blendState);

		//�����p�V�F�[�_
		m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

		//�����p�T���v���[
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
	}

	void BloomRender::Release() {
		if (m_tex) { m_tex->Release(); m_tex = nullptr; }
		if (m_SRV) { m_SRV->Release(); m_SRV = nullptr; }
		if (m_outputUAV) { m_outputUAV->Release(); m_outputUAV = nullptr; }
		if (m_cb) { m_cb->Release(); m_cb = nullptr; }
		if (m_interferenceFringesSRV) { m_interferenceFringesSRV->Release(); m_interferenceFringesSRV = nullptr; }
		if (m_blendState) { m_blendState->Release(); m_blendState = nullptr; }
		if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
	}

	void BloomRender::Render()
	{
		if (!m_enable) { return; }

		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//�`�����N���A
		float clearColor[4] = {
			0.0f, 0.0f, 0.0f, 0.0f
		};
		rc->ClearRenderTargetView(m_RTV, clearColor);		
		
		// �ݒ�
		{
			//�萔�o�b�t�@
			SCSConstantBuffer csCb;
			csCb.win_x = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
			csCb.win_y = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
			csCb.out_x = m_textureSizeX;
			csCb.out_y = m_textureSizeY;
			csCb.luminanceThreshold = m_luminanceThreshold;
			rc->UpdateSubresource(m_cb, 0, nullptr, &csCb, 0, 0);
			rc->CSSetConstantBuffers(0, 1, &m_cb);

			//CS
			rc->CSSetShader((ID3D11ComputeShader*)m_cs.GetBody(), NULL, 0);
			//UAV
			rc->CSSetUnorderedAccessViews(0, 1, &m_outputUAV, nullptr);

			//SRV��ݒ�
			rc->CSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
			rc->CSSetShaderResources(2, 1, &m_interferenceFringesSRV);

			//�T���v���X�e�[�g��ݒ�B
			rc->CSSetSamplers(0, 1, &m_samplerState);
		}

		// �f�B�X�p�b�`
		rc->Dispatch((UINT)std::ceil(m_textureSizeX / 32.0f), (UINT)std::ceil(m_textureSizeY / 32.0f), 1);

		//�ݒ����
		{
			ID3D11Buffer* pCB = NULL;
			rc->CSSetConstantBuffers(0, 1, &pCB);

			ID3D11ShaderResourceView*	pReses = NULL;
			rc->CSSetShaderResources(1, 1, &pReses);
			rc->CSSetShaderResources(2, 1, &pReses);

			ID3D11UnorderedAccessView*	pUAV = NULL;
			rc->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);
		}

		//�ڂ���
		m_gaussBlur.Blur();

		//����

		//�u�����h�X�e�[�g�ύX
		ID3D11BlendState* oldBlendState = nullptr; FLOAT oldf[4]; UINT olduint;
		GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldf, &olduint);

		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);

		//SRV���Z�b�g	
		rc->PSSetShaderResources(0, 1, &m_gaussBlur.GetSRV());
		
		//�`�����ŏI�����_�[�^�[�Q�b�g�ɂ���
		GetEngine().GetGraphicsEngine().SetFinalRenderTarget();		

		//�V�F�[�_�[��ݒ�
		rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
		rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
		//���̓��C�A�E�g��ݒ�B
		rc->IASetInputLayout(m_vs.GetInputLayout());
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
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//�u�����h�X�e�[�g�߂�
		if (oldBlendState) {
			GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldf, olduint);
			oldBlendState->Release();
		}
	}
}