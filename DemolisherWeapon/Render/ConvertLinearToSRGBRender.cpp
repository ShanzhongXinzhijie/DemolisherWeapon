#include "DWstdafx.h"
#include "ConvertLinearToSRGBRender.h"

namespace DemolisherWeapon {

ConvertLinearToSRGBRender::ConvertLinearToSRGBRender()
{
}

ConvertLinearToSRGBRender::~ConvertLinearToSRGBRender()
{
	Release();
}

void ConvertLinearToSRGBRender::Init() {
	m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
	m_ps.Load("Preset/shader/primitive.fx", "PSConvertLinearToSRGB", Shader::EnType::PS);

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
}
void ConvertLinearToSRGBRender::Release() {
	if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
}

void ConvertLinearToSRGBRender::Render() {
	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//SRV���Z�b�g	
	rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
	
	//�ŏI�I�ȃ����_�[�^�[�Q�b�g�̃X���b�v
	GetEngine().GetGraphicsEngine().GetFRT().Swap();

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
}

}