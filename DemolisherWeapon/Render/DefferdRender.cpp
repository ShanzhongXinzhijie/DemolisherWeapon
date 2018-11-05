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

	//サンプラー
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);	
	
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerStateNoFillter);

	//定数バッファ
	int bufferSize = sizeof(ShadowConstantBuffer);
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_scb);

	//SampleCmp用
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 1.0f;
	desc.BorderColor[2] = 1.0f;
	desc.BorderColor[3] = 1.0f;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;//比較方法
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerComparisonState);
}
void DefferdRender::Release() {
	m_samplerState->Release();
	m_samplerStateNoFillter->Release();
	m_scb->Release();
	m_samplerComparisonState->Release();
}

void DefferdRender::Render() {
	
	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//描画先を最終レンダーターゲットにする
	GetEngine().GetGraphicsEngine().GetFinalRender().SetFinalRenderTarget();

	//SRVをセット	
	rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferAlbedo));
	rc->PSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferNormal));
	rc->PSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetDepthStencilSRV());
	rc->PSSetShaderResources(3, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));
	rc->PSSetShaderResources(4, 1, &GetEngine().GetGraphicsEngine().GetAmbientOcclusionRender().GetAmbientOcclusionSRV());
	
	//ライト関係をセット
	rc->PSSetShaderResources(100, 1, &GetEngine().GetGraphicsEngine().GetLightManager().GetDirectionLightSRV());
	rc->PSSetShaderResources(101, 1, &GetEngine().GetGraphicsEngine().GetLightManager().GetPointLightsSRV());
	rc->PSSetConstantBuffers(2, 1, &GetEngine().GetGraphicsEngine().GetLightManager().GetLlightParamCB());

	//シャドウマップ関係をセット
	ShadowConstantBuffer sCb;
	sCb.boolAO = GetEngine().GetGraphicsEngine().GetAmbientOcclusionRender().GetEnable() ? 1 : 0;
	sCb.mViewProjInv.Mul(GetMainCamera()->GetViewMatrix(), GetMainCamera()->GetProjMatrix());
	sCb.mViewProjInv.Inverse(sCb.mViewProjInv);
	for (int i = 0; i < GetEngine().GetGraphicsEngine().GetShadowMapRender().GetSHADOWMAP_NUM(); i++) {
		rc->PSSetShaderResources(60+i, 1, &GetEngine().GetGraphicsEngine().GetShadowMapRender().GetShadowMapSRV(i));
		sCb.mLVP[i] = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightViewProjMatrix(i);
		sCb.shadowDir[i] = GetEngine().GetGraphicsEngine().GetShadowMapRender().GetLightDir(i);
	}
	rc->UpdateSubresource(m_scb, 0, nullptr, &sCb, 0, 0);
	//rc->VSSetConstantBuffers(1, 1, &m_scb);
	rc->PSSetConstantBuffers(1, 1, &m_scb);

	//シェーダーを設定
	rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
	rc->PSSetShader((ID3D11PixelShader*) m_ps.GetBody(), NULL, 0);
	//入力レイアウトを設定。
	rc->IASetInputLayout(m_vs.GetInputLayout());
	//サンプラステートを設定。
	rc->PSSetSamplers(0, 1, &m_samplerState);
	rc->PSSetSamplers(1, 1, &m_samplerComparisonState); 
	rc->PSSetSamplers(2, 1, &m_samplerStateNoFillter);	

	//描画
	GetEngine().GetGraphicsEngine().DrawFullScreen();

	//SRVを解除
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	rc->PSSetShaderResources(0, 1, view);
	rc->PSSetShaderResources(1, 1, view);
	rc->PSSetShaderResources(2, 1, view);
	rc->PSSetShaderResources(3, 1, view);
	rc->PSSetShaderResources(4, 1, view);

	rc->PSSetShaderResources(100, 1, view);
	rc->PSSetShaderResources(101, 1, view);

	for (int i = 0; i < GetEngine().GetGraphicsEngine().GetShadowMapRender().GetSHADOWMAP_NUM(); i++) {
		rc->PSSetShaderResources(60 + i, 1, view);
	}
}

}