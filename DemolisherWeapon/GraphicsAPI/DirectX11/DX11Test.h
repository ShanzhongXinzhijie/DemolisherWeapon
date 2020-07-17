#pragma once
#include "GraphicsAPI/IGraphicsAPI.h"

namespace DemolisherWeapon {

	class DX11Test : public IGraphicsAPI
	{
	public:
		bool Init(HWND hWnd, const InitEngineParameter& initParam)override;
		void Release()override;

		void ExecuteCommand() {}

		/// <summary>
		/// フレームバッファサイズの変更(再設定)
		/// </summary>
		void ChangeFrameBufferSize()override;

		/// <summary>
		/// D3D11デバイスを取得
		/// </summary>
		ID3D11Device* GetD3DDevice()
		{
			return m_pd3dDevice;
		}

		/// <summary>
		/// D3D11デバイスコンテキストを取得
		/// </summary>
		ID3D11DeviceContext* GetD3DDeviceContext()
		{
			return m_pd3dDeviceContext;
		}

		/// <summary>
		/// ラスタライザーステートをリセット
		/// </summary>
		void ResetRasterizerState() {
			m_pd3dDeviceContext->RSSetState(m_rasterizerState);
		}

		/// <summary>
		/// バックバッファのクリア
		/// </summary>
		void ClearBackBuffer()override
		{
			//バックバッファを灰色で塗りつぶす。
			float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
			m_pd3dDeviceContext->ClearRenderTargetView(m_backBuffer, ClearColor);
			//デプスのクリア
			m_pd3dDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}

		/// <summary>
		/// バックバッファをレンダーターゲットにする
		/// </summary>
		void SetBackBufferToRenderTarget()override;

		/// <summary>
		/// バックバッファのスワップ
		/// </summary>
		void SwapBackBuffer()override;

		/// <summary>
		/// ビューポートの設定
		/// </summary>
		void SetViewport(float topLeftX, float topLeftY, float width, float height)override
		{
			m_viewport.Width = width;
			m_viewport.Height = height;
			m_viewport.TopLeftX = topLeftX;
			m_viewport.TopLeftY = topLeftY;
			m_viewport.MinDepth = 0.0f;
			m_viewport.MaxDepth = 1.0f;
			m_pd3dDeviceContext->RSSetViewports(1, &m_viewport);
		}

	private:
		ID3D11Device* m_pd3dDevice = nullptr;					//D3D11デバイス。
		ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;		//D3D11デバイスコンテキスト。
		IDXGISwapChain* m_pSwapChain = nullptr;					//スワップチェイン。
		ID3D11RenderTargetView* m_backBuffer = nullptr;			//バックバッファ。
		ID3D11RasterizerState* m_rasterizerState = nullptr;		//ラスタライザステート。
		ID3D11Texture2D* m_depthStencil = nullptr;				//デプスステンシル。
		ID3D11DepthStencilView* m_depthStencilView = nullptr;	//デプスステンシルビュー。
		ID3D11DepthStencilState* m_depthStencilState = nullptr;
		
		D3D11_VIEWPORT m_viewport;//ビューポート
	};

}

