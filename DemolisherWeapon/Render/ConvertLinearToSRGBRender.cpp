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

	//SRVをセット	
	rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
	
	//最終的なレンダーターゲットのスワップ
	GetEngine().GetGraphicsEngine().GetFRT().Swap();

	//描画先を最終レンダーターゲットにする
	GetEngine().GetGraphicsEngine().SetFinalRenderTarget();

	//シェーダーを設定
	rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
	rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
	//入力レイアウトを設定。
	rc->IASetInputLayout(m_vs.GetInputLayout());
	//サンプラステートを設定。
	rc->PSSetSamplers(0, 1, &m_samplerState);

	//描画
	GetEngine().GetGraphicsEngine().DrawFullScreen();

	//SRVを解除
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	rc->PSSetShaderResources(0, 1, view);

	//レンダーターゲット解除
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}

}