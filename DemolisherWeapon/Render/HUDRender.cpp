#include "DWstdafx.h"
#include "HUDRender.h"

namespace DemolisherWeapon {
	void HUDRender::Init(int HUDNum, const CVector2& screen_min, const CVector2& screen_max, const CVector2& screenSize) {
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		m_HUDNum = HUDNum;
		m_textureSize = screenSize;

		//出力テクスチャDESC
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = static_cast<UINT>(screenSize.x);
		texDesc.Height = static_cast<UINT>(screenSize.y);
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		//出力テクスチャの作成
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_texture.ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_texture.Get(), nullptr, m_RTV.ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_texture.Get(), nullptr, m_SRV.ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//頂点シェーダ
		m_vs.Load("Preset/shader/primitive.fx", "VSMain", Shader::EnType::VS);
		//ピクセルシェーダ
		m_ps.Load("Preset/shader/primitive.fx", "PSMain", Shader::EnType::PS);

		//描画図形	
		m_vertex[0] = {
			{screen_min.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		};
		m_vertex[1] = {
			{screen_max.x*2.0f - 1.0f, screen_min.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		};
		m_vertex[2] = {
			{screen_min.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		};
		m_vertex[3] = {
			{screen_max.x*2.0f - 1.0f, screen_max.y*2.0f - 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		};
		m_drawSpace.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, m_vertex, 4, m_index);
	}
	void HUDRender::Render() {
		//ビューポート設定
		D3D11_VIEWPORT oldviewport; UINT kaz = 1;
		GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &oldviewport);
		//ブレンドステート設定
		ID3D11BlendState* oldBlendState = nullptr; FLOAT oldf[4]; UINT olduint;
		GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldf, &olduint);

	//HUDに描画
		//フレームバッファサイズの変更
		GetGraphicsEngine().ChangeFrameBufferSize(m_textureSize.x, m_textureSize.y);

		//レンダーターゲットのクリア
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_RTV.Get(), clearColor);
		//レンダーターゲットの設定
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr);
		//ビューポート
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

		//2Dプリミティブの描画
		GetGraphicsEngine().GetPrimitiveRender().RenderHUD(m_HUDNum);
		GetGraphicsEngine().GetPrimitiveRender().PostRenderHUD(m_HUDNum);
		//描画
		GetEngine().GetGameObjectManager().HUDRender(m_HUDNum);

		//フレームバッファサイズを戻す
		GetGraphicsEngine().ResetFrameBufferSize();

	//バックバッファにHUDを描画
		//描画先をバックバッファにする
		GetGraphicsEngine().SetBackBufferToRenderTarget();

		//2D用の設定にする
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

		//SRVをセット
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_SRV.GetAddressOf());

		//シェーダーを設定
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);
		//入力レイアウトを設定。
		GetGraphicsEngine().GetD3DDeviceContext()->IASetInputLayout(m_vs.GetInputLayout());
		//サンプラステートを設定。
		auto samp = GetGraphicsEngine().GetCommonStates().LinearClamp();
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetSamplers(0, 1, &samp);
		//ブレンドステートを設定
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().AlphaBlend(), nullptr, 0xFFFFFFFF);
		//ラスタライザーステートを設定
		GetGraphicsEngine().ResetRasterizerState();

		//描画
		m_drawSpace.DrawIndexed();

		//SRVを解除
		ID3D11ShaderResourceView* view[] = { NULL };
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(0, 1, view);

		//ビューポート戻す
		GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &oldviewport);
		//ブレンドステート戻す
		if (oldBlendState) {
			GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldf, olduint);
			oldBlendState->Release();
		}
	}
}