#include "DWstdafx.h"
#include "DefferdRender.h"

namespace DemolisherWeapon {

DefferdRender::DefferdRender()
{	
}
DefferdRender::~DefferdRender()
{
	Release();
}

void DefferdRender::Init() {
	m_vs.Load("Preset/shader/defferd.fx", "VSMain", Shader::EnType::VS);
	m_ps.Load("Preset/shader/defferd.fx", "PSMain", Shader::EnType::PS);

	//�T���v���[
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.MaxLOD = FLT_MAX;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
	
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerStateNoFillter);

	//�萔�o�b�t�@
	int bufferSize = sizeof(ShadowConstantBuffer);
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_scb);

	//SampleCmp�p
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 2.0f;
	desc.BorderColor[1] = 2.0f;
	desc.BorderColor[2] = 2.0f;
	desc.BorderColor[3] = 2.0f;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;//��r���@
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerComparisonState);
}
void DefferdRender::Release() {
	m_samplerState->Release();
	m_samplerStateNoFillter->Release();
	m_scb->Release();
	m_samplerComparisonState->Release();
	if (m_ambientCube) { m_ambientCube->Release(); m_ambientCube = nullptr; }
}

void DefferdRender::Render() {
	
	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "�J�������ݒ肳��Ă��܂���!!", "Error", MB_OK);
		std::abort();
	}
#endif

	//�`�����ŏI�����_�[�^�[�Q�b�g�ɂ���
	GetEngine().GetGraphicsEngine().SetFinalRenderTarget_NonDepth();

	//SRV���Z�b�g	
	rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferAlbedo));
	rc->PSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferNormal));
	rc->PSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetDepthStencilSRV());
	rc->PSSetShaderResources(3, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));
	rc->PSSetShaderResources(4, 1, &GetEngine().GetGraphicsEngine().GetAmbientOcclusionRender().GetAmbientOcclusionSRV());
	rc->PSSetShaderResources(5, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferLightParam));
	rc->PSSetShaderResources(7, 1, &GetEngine().GetGraphicsEngine().GetAmbientOcclusionRender().GetAmbientOcclusionBlurSRV());

	//���C�g�֌W���Z�b�g
	rc->PSSetShaderResources(100, 1, &GetEngine().GetGraphicsEngine().GetLightManager().GetDirectionLightSRV());
	rc->PSSetShaderResources(101, 1, &GetEngine().GetGraphicsEngine().GetLightManager().GetPointLightsSRV());
	rc->PSSetConstantBuffers(2, 1, &GetEngine().GetGraphicsEngine().GetLightManager().GetLlightParamCB());

	//�V���h�E�}�b�v�֌W���Z�b�g
	ShadowConstantBuffer sCb;

	//AO��L���ɂ��邩
	sCb.boolAO = GetEngine().GetGraphicsEngine().GetAmbientOcclusionRender().GetEnable() ? 1 : 0;
	//���L���[�u�}�b�v�L���ɂ��邩
	sCb.boolAmbientCube = m_isAmbCube ? 1 : 0;
	if (sCb.boolAmbientCube) {
		//���L���[�u�}�b�v�Z�b�g
		rc->PSSetShaderResources(6, 1, &m_ambientCube);
	}

	//���C���J�����̋t�s��
	sCb.mViewProjInv.Mul(GetMainCamera()->GetViewMatrix(), GetMainCamera()->GetProjMatrix());
	sCb.mViewProjInv.Inverse(sCb.mViewProjInv);

	//�V�F�[�_�[���\�[�X�ݒ�
	rc->PSSetShaderResources(60, 1, &GetEngine().GetGraphicsEngine().GetShadowMapRender().GetShadowMapSRV());

	for (int i = 0; i < ShadowMapRender::SHADOWMAP_NUM; i++) {
		//�L�����H
		sCb.enableShadowMap[i].x = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetShadowMapEnable(i) ? 1.0f : 0.0f;
		//PCSS
		sCb.enableShadowMap[i].y = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetEnablePCSS(i) ? 1.0f : 0.0f;
		//�V���h�E�}�b�v�𑜓x
		sCb.enableShadowMap[i].z = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetSizeX(i);
		sCb.enableShadowMap[i].w = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetSizeY(i);

		if (sCb.enableShadowMap[i].x) {
			//�萔
			sCb.mLVP[i] = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightViewProjMatrix(i);
			sCb.shadowDir[i] = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightDir(i);
			sCb.shadowDir[i].w = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetDepthBias(i);

			//�J�X�P�[�h�͈̔�
			sCb.cascadeArea[i].x = GetMainCamera()->GetFar() * GetEngine().GetGraphicsEngine().GetShadowMapRender().GetCascadeNear(i);
			sCb.cascadeArea[i].y = GetMainCamera()->GetFar() * GetEngine().GetGraphicsEngine().GetShadowMapRender().GetCascadeFar(i);
		
			//���s���e�J���� Width�EHeight
			sCb.cascadeArea[i].z = 4000.0f / GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightCamera(i).GetWidth();
			sCb.cascadeArea[i].w = 4000.0f / GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightCamera(i).GetHeight();
		
			//Near,Far
			sCb.shadowNF[i].x = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightCamera(i).GetNear();
			sCb.shadowNF[i].y = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightCamera(i).GetFar();
			sCb.shadowNF[i].z = sCb.shadowNF[i].y - sCb.shadowNF[i].x;
		}
	}
	rc->UpdateSubresource(m_scb, 0, nullptr, &sCb, 0, 0);
	rc->PSSetConstantBuffers(1, 1, &m_scb);

	//�V�F�[�_�[��ݒ�
	rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
	rc->PSSetShader((ID3D11PixelShader*) m_ps.GetBody(), NULL, 0);
	//���̓��C�A�E�g��ݒ�B
	rc->IASetInputLayout(m_vs.GetInputLayout());
	//�T���v���X�e�[�g��ݒ�B
	rc->PSSetSamplers(0, 1, &m_samplerState);
	rc->PSSetSamplers(1, 1, &m_samplerComparisonState); 
	rc->PSSetSamplers(2, 1, &m_samplerStateNoFillter);	

	//�`��
	GetEngine().GetGraphicsEngine().DrawFullScreen();

	//SRV������
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	rc->PSSetShaderResources(0, 1, view);
	rc->PSSetShaderResources(1, 1, view);
	rc->PSSetShaderResources(2, 1, view);
	rc->PSSetShaderResources(3, 1, view);
	rc->PSSetShaderResources(4, 1, view);
	rc->PSSetShaderResources(5, 1, view);
	rc->PSSetShaderResources(6, 1, view);
	rc->PSSetShaderResources(7, 1, view);

	rc->PSSetShaderResources(100, 1, view);
	rc->PSSetShaderResources(101, 1, view);

	for (int i = 0; i < ShadowMapRender::SHADOWMAP_NUM; i++) {
		rc->PSSetShaderResources(60 + i, 1, view);
	}

	//�����_�[�^�[�Q�b�g����
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}

void DefferdRender::SetAmbientCubeMap(const wchar_t* filePass) {

	ID3D11ShaderResourceView* tex = nullptr;
	HRESULT hr = DirectX::CreateDDSTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), filePass, nullptr, &tex);
	if (FAILED(hr)) {
		Error::Box("SetAmbientCubeMap�̃e�N�X�`���ǂݍ��݂Ɏ��s���܂���");
		return;
	}

	if (m_ambientCube) { m_ambientCube->Release(); }
	m_ambientCube = tex;
}

}