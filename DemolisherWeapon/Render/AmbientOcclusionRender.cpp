#include "DWstdafx.h"
#include "AmbientOcclusionRender.h"

namespace DemolisherWeapon {

AmbientOcclusionRender::AmbientOcclusionRender()
{
}
AmbientOcclusionRender::~AmbientOcclusionRender()
{
	Release();
}

void AmbientOcclusionRender::Init(float texScale) {

	GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

	m_textureSizeX = (UINT)(ge.GetFrameBuffer_W()*texScale);
	m_textureSizeY = (UINT)(ge.GetFrameBuffer_H()*texScale);

	//�R���s���[�g�V�F�[�_
	m_cs.Load("Preset/shader/ambientocclusionCS.fx", "CSmain", Shader::EnType::CS);

	//�e�N�X�`���쐬
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = m_textureSizeX;
	texDesc.Height = m_textureSizeY;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_ambientOcclusionTex);
	ge.GetD3DDevice()->CreateShaderResourceView(m_ambientOcclusionTex, nullptr, &m_ambientOcclusionSRV);//�V�F�[�_�[���\�[�X�r���[

	//OutPutUAV
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC	uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));

		uavDesc.Format = texDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		HRESULT	hr;
		hr = ge.GetD3DDevice()->CreateUnorderedAccessView(m_ambientOcclusionTex, &uavDesc, &m_outputUAV);
	}

	//�萔�o�b�t�@
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((sizeof(SCSConstantBuffer) - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	ge.GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cb);
}
void AmbientOcclusionRender::Release() {
	m_ambientOcclusionTex->Release();
	m_outputUAV->Release();
	m_cb->Release();
}

void AmbientOcclusionRender::Render() {

	if (!m_enable) { return; }

	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();
	
	// �ݒ�
	{
		//�萔�o�b�t�@
		SCSConstantBuffer csCb;
		csCb.win_x = (UINT)GetEngine().GetGraphicsEngine().GetFrameBuffer_W();
		csCb.win_y = (UINT)GetEngine().GetGraphicsEngine().GetFrameBuffer_H();
		csCb.ao_x = m_textureSizeX;
		csCb.ao_y = m_textureSizeY;
		csCb.distanceScale = GetEngine().GetDistanceScale();
		csCb.mViewProjInv.Mul(GetMainCamera()->GetViewMatrix(), GetMainCamera()->GetProjMatrix());
		csCb.mViewProjInv.Inverse(csCb.mViewProjInv);
		rc->UpdateSubresource(m_cb, 0, nullptr, &csCb, 0, 0);
		rc->CSSetConstantBuffers(0, 1, &m_cb);

		//CS
		rc->CSSetShader((ID3D11ComputeShader*)m_cs.GetBody(), NULL, 0);
		//UAV
		rc->CSSetUnorderedAccessViews(0, 1, &m_outputUAV, nullptr);
		
		//SRV��ݒ�
		rc->CSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferNormal));
		rc->CSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));
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
}

}