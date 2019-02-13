#include "DWstdafx.h"
#include "FinalRender.h"

namespace DemolisherWeapon {

CFinalRenderTarget::~CFinalRenderTarget() {
	Release();
}
void CFinalRenderTarget::Init() {
	//テクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W();
	texDesc.Height = (UINT)GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	for (int i = 0; i < 2; i++) {
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_Tex[i]);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateRenderTargetView(m_Tex[i], nullptr, &m_RTV[i]);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_Tex[i], nullptr, &m_SRV[i]);
	}

	//デプスステンシル作成
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateTexture2D(&texDesc, NULL, &m_depthStencilTex);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	ZeroMemory(&dsv_desc, sizeof(dsv_desc));
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex, &dsv_desc, &m_depthStencilView);
}
void CFinalRenderTarget::Release() {
	for (int i = 0; i < 2; i++) {
		if (m_Tex[i]) { m_Tex[i]->Release(); m_Tex[i] = nullptr; }
		if (m_RTV[i]) { m_RTV[i]->Release(); m_RTV[i] = nullptr; }
		if (m_SRV[i]) { m_SRV[i]->Release(); m_SRV[i] = nullptr; }
	}
	if (m_depthStencilTex) { m_depthStencilTex->Release(); m_depthStencilTex = nullptr; }
	if (m_depthStencilView) { m_depthStencilView->Release(); m_depthStencilView = nullptr; }
}
void CFinalRenderTarget::Clear(int num) {
	int n = num;
	if (num == -1) { n = m_now; }

	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_RTV[n], ClearColor);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void CFinalRenderTarget::AllClear() {
	for (int i = 0; i < 2; i++) {
		Clear(i);
	}
}
void CFinalRenderTarget::Copy() {
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->CopyResource(m_Tex[m_now^1], m_Tex[m_now]);
}

//ファイナルレンダー
FinalRender::~FinalRender()
{
	Release();
}
void FinalRender::Release() {
	//m_FRT.Release();
	if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
	if (m_cb) { m_cb->Release(); m_cb = nullptr; }

	if (m_gridTex) { m_gridTex->Release(); m_gridTex = nullptr; }
}
void FinalRender::Init(const CVector2 screen_min, const CVector2 screen_max) {
	//m_FRT.Init();

	m_vs.Load("Preset/shader/LensDistortion.fx", "VSMain", Shader::EnType::VS);
	m_ps.Load("Preset/shader/LensDistortion.fx", "PSMain", Shader::EnType::PS);

	m_psNormal.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);
	
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
	
	//定数バッファ
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (((sizeof(SPSConstantBuffer) - 1) / 16) + 1) * 16;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &m_cb);

	CPrimitive::SVertex vertex[4] = {
		{
			{screen_min.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		},
		{
			{screen_max.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		},
		{
			{screen_min.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		},
		{
			{screen_max.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		},
	};
	int index[4] = { 0,1,2,3 };
	m_drawSpace.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, 4, index);
}
void FinalRender::Render() {
	//描画先をバックバッファにする
	GetEngine().GetGraphicsEngine().SetBackBufferToRenderTarget();

	//ビューポート設定
	D3D11_VIEWPORT oldviewport; UINT kaz = 1;
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
	GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

	ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//SRVをセット
	if (GetIsDebugInput() && GetAsyncKeyState(VK_NUMPAD0)) {
		if (!m_gridTex) {
			HRESULT hr = DirectX::CreateWICTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), L"Asset/modelData/grid.png", nullptr, &m_gridTex, 0);
		}
		rc->PSSetShaderResources(0, 1, &m_gridTex);
	}
	else {
		rc->PSSetShaderResources(0, 1, &GetEngine().GetGraphicsEngine().GetFRT().GetSRV());
	}

	//シェーダーを設定
	rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
	if (1){//GetIsDebugInput() && GetAsyncKeyState(VK_NUMPAD1)) {
		rc->PSSetShader((ID3D11PixelShader*)m_psNormal.GetBody(), NULL, 0);
	}
	else {
		rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
	}
	//入力レイアウトを設定。
	rc->IASetInputLayout(m_vs.GetInputLayout());
	//サンプラステートを設定。
	rc->PSSetSamplers(0, 1, &m_samplerState);

	//定数バッファ
	SPSConstantBuffer psCb;
	float fov = GetMainCamera()->GetFOV(); 
	
	if (fov < 0.0f) { 
		//平行
		psCb.k4 = 0.0f;
		psCb.LENS_DISTORTION_UV_MAGNIFICATION = 1.0f;
	}
	else {
		//透視
		//psCb.k4 = -2.0f;//-0.0875f * (fov / (3.14f*0.5f)); 
		if (fov > 3.14f*0.5f) {
			psCb.k4 = CMath::Lerp(pow((fov - (3.14f*0.5f)) / (CMath::DegToRad(160.4f) - (3.14f*0.5f)),2.0f), -0.0875f, -2.0f);
		}
		else {
			psCb.k4 = min(0.0f, CMath::Lerp((((3.14f*0.5f) - fov) / (3.14f*0.5f))*4.0f, -0.0875f, 0.0f));
		}

		// Aspect比を考慮した空間上での,光軸中心からの距離ベクトル
		CVector3 rVec = (CVector3(1.0f,0.0f,0.0f) - CVector3(0.5f, 0.5f, 0.0f));
		rVec.x *= GetMainCamera()->GetAspect();
		const float r2 = rVec.Dot(rVec);
		const float r4 = r2 * r2;

		// 半径方向歪みを考慮した距離ベクトル
		CVector3 distortionR = rVec * ((1.0f) / (1.0f + psCb.k4 * r4));

		// アスペクト比補正
		distortionR.x *= 1.0f / GetMainCamera()->GetAspect();

		// 画面中心基準の拡大,UV座標への変換
		CVector3 uv2 = (distortionR) + CVector3(0.5f, 0.5f, 0.0f);		
		psCb.LENS_DISTORTION_UV_MAGNIFICATION = (1.0f / uv2.x);
	}

	psCb.ASPECT_RATIO = GetMainCamera()->GetAspect();
	psCb.INV_ASPECT_RATIO = 1.0f / GetMainCamera()->GetAspect();

	rc->UpdateSubresource(m_cb, 0, nullptr, &psCb, 0, 0);
	rc->PSSetConstantBuffers(0, 1, &m_cb);

	//描画
	m_drawSpace.DrawIndexed();
	//GetEngine().GetGraphicsEngine().DrawFullScreen();

	//定数バッファ解除
	ID3D11Buffer* pCB = NULL;
	rc->PSSetConstantBuffers(0, 1, &pCB);

	//SRVを解除
	ID3D11ShaderResourceView* view[] = {
				NULL
	};
	rc->PSSetShaderResources(0, 1, view);

	//最終レンダーターゲットをクリア
	GetEngine().GetGraphicsEngine().GetFRT().AllClear();

	//ビューポート戻す
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);
}
//void FinalRender::SetFinalRenderTarget() {
//	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), m_FRT.GetDSV());
//}

}