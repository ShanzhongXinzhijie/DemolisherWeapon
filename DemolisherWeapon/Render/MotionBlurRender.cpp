#include "DWstdafx.h"
#include "MotionBlurRender.h"

namespace DemolisherWeapon {

MotionBlurRender::MotionBlurRender()
{
}
MotionBlurRender::~MotionBlurRender()
{
	Release();
}

void MotionBlurRender::Init() {

	GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

	//コンピュートシェーダ
	m_cs.Load("Preset/shader/motionblurCS.fx", "CSmain", Shader::EnType::CS);
	//ピクセルシェーダ
	m_vs.Load("Preset/shader/motionblurPS.fx", "VSMain", Shader::EnType::VS);
	m_ps.Load("Preset/shader/motionblurPS.fx", "PSMain", Shader::EnType::PS);
	
	//OutPutUAV
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC	uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));

		D3D11_TEXTURE2D_DESC texDesc;
		ge.GetFRT().GetTex(0)->GetDesc(&texDesc);

		uavDesc.Format = texDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		HRESULT	hr;
		hr = ge.GetD3DDevice()->CreateUnorderedAccessView(ge.GetFRT().GetTex(0), &uavDesc, &m_outputUAV);
	}

	//MaskUAV
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC	uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));

		D3D11_TEXTURE2D_DESC texDesc;
		ge.GetGBufferRender().GetGBufferTex(GBufferRender::enGBufferVelocity)->GetDesc(&texDesc);

		uavDesc.Format = texDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		HRESULT	hr;
		hr = ge.GetD3DDevice()->CreateUnorderedAccessView(ge.GetGBufferRender().GetGBufferTex(GBufferRender::enGBufferVelocity), &uavDesc, &m_maskUAV);
	}

	//サンプラー
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;// LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);

	//定数バッファ
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((sizeof(SCSConstantBuffer) - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	
	bufferDesc.CPUAccessFlags = 0;
	ge.GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cb);

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((sizeof(SPSConstantBuffer) - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	ge.GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cbPS);
}
void MotionBlurRender::Release() {
	m_outputUAV->Release();
	m_maskUAV->Release();
	m_cb->Release();
	m_cbPS->Release();
	m_samplerState->Release();
}

void MotionBlurRender::Render() {

	if (!m_isEnable) { return; }

	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();
	PSBlur(rc);
	CSBlur(rc);
}

void MotionBlurRender::PSBlur(ID3D11DeviceContext* rc){		

	//最終レンダーターゲットのコピー
	GetEngine().GetGraphicsEngine().GetFRT().Copy();

	//SRVをセット	
	rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV(1));
	rc->PSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferVelocityPS));

	//最終的なレンダーターゲットのスワップ
	GetEngine().GetGraphicsEngine().GetFRT().SetNow(0);// Swap();
	//描画先を最終レンダーターゲットにする
	//GetEngine().GetGraphicsEngine().GetFRT().Clear(0);
	GetEngine().GetGraphicsEngine().SetFinalRenderTarget();

	//定数バッファ
	SPSConstantBuffer psCb;
	psCb.DistantThreshold = 500.0f*GetEngine().GetDistanceScale();
	rc->UpdateSubresource(m_cbPS, 0, nullptr, &psCb, 0, 0);
	rc->PSSetConstantBuffers(0, 1, &m_cbPS);

	//シェーダーを設定
	rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
	rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
	//入力レイアウトを設定。
	rc->IASetInputLayout(m_vs.GetInputLayout());
	//サンプラステートを設定。
	rc->PSSetSamplers(0, 1, &m_samplerState);

	//描画
	GetEngine().GetGraphicsEngine().DrawFullScreen();

	//定数バッファ解除
	ID3D11Buffer* pCB = NULL;
	rc->PSSetConstantBuffers(0, 1, &pCB);

	//SRVを解除
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	rc->PSSetShaderResources(0, 1, view);
	rc->PSSetShaderResources(1, 1, view);

	//レンダーターゲット解除
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
}

void MotionBlurRender::CSBlur(ID3D11DeviceContext* rc) {
	// 設定
	{
		//定数バッファ
		SCSConstantBuffer csCb;
		csCb.win_x = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
		csCb.win_y = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
		csCb.distanceScale = GetEngine().GetDistanceScale();
		rc->UpdateSubresource(m_cb, 0, nullptr, &csCb, 0, 0);
		rc->CSSetConstantBuffers(0, 1, &m_cb);

		//CS
		rc->CSSetShader((ID3D11ComputeShader*)m_cs.GetBody(), NULL, 0);
		//UAV
		rc->CSSetUnorderedAccessViews(0, 1, &m_outputUAV, nullptr);// (UINT*)&m_outputUAV);
		rc->CSSetUnorderedAccessViews(1, 1, &m_maskUAV, nullptr);

		//SRVを設定
		GetEngine().GetGraphicsEngine().GetFRT().Copy();
		GetEngine().GetGraphicsEngine().GetFRT().SetNow(0);
		rc->CSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV(1));
	}

	// ディスパッチ
	rc->Dispatch((UINT)std::ceil(GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W() / 32.0f), (UINT)std::ceil(GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H() / 32.0f), 1);

	//設定解除
	{
		ID3D11Buffer* pCB = NULL;
		rc->CSSetConstantBuffers(0, 1, &pCB);

		ID3D11ShaderResourceView*	pReses = NULL;
		rc->CSSetShaderResources(2, 1, &pReses);
		
		ID3D11UnorderedAccessView*	pUAV = NULL;
		rc->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);// (UINT*)&pUAV);
		rc->CSSetUnorderedAccessViews(1, 1, &pUAV, nullptr);
	}
}

}