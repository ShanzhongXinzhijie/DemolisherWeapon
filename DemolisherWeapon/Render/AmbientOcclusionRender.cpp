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

	m_texScale = texScale;
	m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W()*texScale);
	m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H()*texScale);

	//コンピュートシェーダ
	m_cs.Load("Preset/shader/ambientocclusionCS.fx", "CSmain", Shader::EnType::CS);

	//テクスチャ作成
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
	ge.GetD3DDevice()->CreateShaderResourceView(m_ambientOcclusionTex, nullptr, &m_ambientOcclusionSRV);//シェーダーリソースビュー

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

	//定数バッファ
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((sizeof(SCSConstantBuffer) - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	ge.GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cb);

	//ガウスブラー
	m_gaussBlur.Init(m_ambientOcclusionSRV, 0.25f, {1280.0f*0.5f,720.0f*0.5f });
}
void AmbientOcclusionRender::Release() {
	m_ambientOcclusionTex->Release();
	m_outputUAV->Release();
	m_cb->Release();
}

void AmbientOcclusionRender::Resize() {
	GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

	m_textureSizeX = (UINT)(ge.Get3DFrameBuffer_W()*m_texScale);
	m_textureSizeY = (UINT)(ge.Get3DFrameBuffer_H()*m_texScale);

	//テクスチャ再作成
	D3D11_TEXTURE2D_DESC texDesc;
	m_ambientOcclusionTex->GetDesc(&texDesc);
	texDesc.Width = m_textureSizeX;
	texDesc.Height = m_textureSizeY;
	m_ambientOcclusionTex->Release(); m_ambientOcclusionTex = nullptr;
	m_ambientOcclusionSRV->Release(); m_ambientOcclusionSRV = nullptr;
	ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_ambientOcclusionTex);
	ge.GetD3DDevice()->CreateShaderResourceView(m_ambientOcclusionTex, nullptr, &m_ambientOcclusionSRV);

	//OutPutUAV再作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	m_outputUAV->GetDesc(&uavDesc);
	m_outputUAV->Release(); m_outputUAV = nullptr;
	ge.GetD3DDevice()->CreateUnorderedAccessView(m_ambientOcclusionTex, &uavDesc, &m_outputUAV);	

	//ガウスブラー
	m_gaussBlur.ResetSource(m_ambientOcclusionSRV);
}

void AmbientOcclusionRender::Render() {

	if (!m_enable) { return; }

	//GPUイベントの開始
	GetGraphicsEngine().BeginGPUEvent(L"AmbientOcclusionRender");

	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "カメラが設定されていません!!", "Error", MB_OK);
		std::abort();
	}
#endif
	
	// 設定
	{
		//定数バッファ
		SCSConstantBuffer csCb;
		csCb.win_x = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
		csCb.win_y = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
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
		
		//SRVを設定
		//rc->CSSetShaderResources(1, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferNormal));
		rc->CSSetShaderResources(2, 1, &GetEngine().GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));
	}

	// ディスパッチ
	rc->Dispatch((UINT)std::ceil(m_textureSizeX / 32.0f), (UINT)std::ceil(m_textureSizeY / 32.0f), 1);

	//設定解除
	{
		ID3D11Buffer* pCB = NULL;
		rc->CSSetConstantBuffers(0, 1, &pCB);

		ID3D11ShaderResourceView*	pReses = NULL;
		//rc->CSSetShaderResources(1, 1, &pReses);
		rc->CSSetShaderResources(2, 1, &pReses);

		ID3D11UnorderedAccessView*	pUAV = NULL;
		rc->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);
	}

	m_gaussBlur.Blur();

	//GPUイベントの終了
	GetGraphicsEngine().EndGPUEvent();
}

ID3D11ShaderResourceView*& AmbientOcclusionRender::GetAmbientOcclusionSRV() {
	return m_ambientOcclusionSRV;
}
ID3D11ShaderResourceView*& AmbientOcclusionRender::GetAmbientOcclusionBlurSRV(){
	return m_gaussBlur.GetSRV();
}

}