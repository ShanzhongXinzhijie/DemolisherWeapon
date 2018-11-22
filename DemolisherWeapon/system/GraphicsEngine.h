#pragma once

#include"Render/IRander.h"
#include"Render/GBufferRender.h"
#include"Render/ShadowMapRender.h"
#include"Render/AmbientOcclusionRender.h"
#include"Render/DefferdRender.h"
#include"Render/FinalRender.h"
#include"Render/MotionBlurRender.h"
#include"Render/PhysicsDebugDrawRender.h"
#include"Render/CameraSwitchRender.h"
#include"Render/EffekseerRender.h"

#include"Camera/CameraManager.h"
#include"Graphic/Light/Lights.h"
#include"Graphic/CPrimitive.h"

namespace DemolisherWeapon {

struct InitEngineParameter;

class GraphicsEngine
{
public:
	GraphicsEngine();
	~GraphicsEngine();

	/*!
	 *@brief	初期化。
	 *@param[in]	hWnd		ウィンドウハンドル。
	 */
	void Init(HWND hWnd, const InitEngineParameter& initParam);
	/*!
	 *@brief	解放。
	 */
	void Release();
	/*!
	 *@brief	D3D11デバイスを取得。
	 */
	ID3D11Device* GetD3DDevice()
	{
		return m_pd3dDevice;
	}
	/*!
	 *@brief	D3D11デバイスコンテキストを取得。
	 */
	ID3D11DeviceContext* GetD3DDeviceContext()
	{
		return m_pd3dDeviceContext;
	}

	//フレームバッファの取得
	float GetFrameBuffer_W()const {
		return FRAME_BUFFER_W;
	}
	float GetFrameBuffer_H()const {
		return FRAME_BUFFER_H;
	}
	float Get3DFrameBuffer_W()const{
		return FRAME_BUFFER_3D_W;
	}
	float Get3DFrameBuffer_H()const {
		return FRAME_BUFFER_3D_H;
	}

	//垂直同期待つかを設定
	void SetUseVSync(bool b) {
		m_useVSync = b;
	}

	//ラスタライザーステートをリセット
	void ResetRasterizerState() {
		m_pd3dDeviceContext->RSSetState(m_rasterizerState);
	}

	//バックバッファをクリア
	void ClearBackBuffer();
	//バックバッファをレンダーターゲットに設定
	void SetBackBufferToRenderTarget();
	//バックバッファとフロントバッファを入れ替える
	void SwapBackBuffer();
	
	//レンダーマネージャーの描画
	void RunRenderManager();

	//Sprite取得
	DirectX::SpriteBatch* GetSpriteBatch() const
	{
		return m_spriteBatch.get();
	}
	DirectX::SpriteFont* GetSpriteFont() const
	{
		return m_spriteFont.get();
	}

	//フルスクリーン描画
	void DrawFullScreen() {
		m_fullscreen.DrawIndexed();
	}

	//3Dモデルレンダーに描画するモデルを登録
	void AddDrawModelToD3Render(SkinModel* sm) {
		m_gbufferRender.AddDrawModel(sm);//m_d3render.AddDrawModel(sm);
	};
	void AddDrawModelToShadowMapRender(SkinModel* sm) {
		m_shadowMapRender.AddDrawModel(sm);
	}

	//Gバッファレンダーの取得
	GBufferRender& GetGBufferRender() {
		return m_gbufferRender;
	}
	//シャドウマップレンダーの取得
	ShadowMapRender& GetShadowMapRender() {
		return m_shadowMapRender;
	}
	//アンビエントオクルージョンレンダーの取得
	AmbientOcclusionRender& GetAmbientOcclusionRender() {
		return m_ambientOcclusionRender;
	}
	//PhysicsDebugDrawRenderのモード設定	
	void SetPhysicsDebugDrawMode(int debugMode) {
#ifndef DW_MASTER	
		m_physicsDebugDrawRender.SetDebugMode(debugMode);
#endif
	}
	//PhysicsDebugDrawRenderが有効か取得
	bool GetEnablePhysicsDebugDraw() {
#ifndef DW_MASTER	
		return m_physicsDebugDrawRender.IsEnable();
#endif
		return false;
	}
	//ファイナルレンダーの取得
	//FinalRender& GetFinalRender() {
	//	return m_finalRender;
	//}

	//カメラマネージャー取得
	CameraManager& GetCameraManager(){
		return m_cameraManager;
	}

	//ライトマネージャー取得
	LightManager& GetLightManager() {
		return m_lightManager;
	}

	//描画先を最終レンダーターゲットに
	void SetFinalRenderTarget();
	//最終レンダーターゲット取得
	CFinalRenderTarget& GetFRT() { return m_FRT; }

	//ビューポート設定
	void SetViewport(float topLeftX, float topLeftY, float width, float height)
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

	float FRAME_BUFFER_W = 1280.0f;				//フレームバッファの幅。
	float FRAME_BUFFER_H = 720.0f;				//フレームバッファの高さ。
	float FRAME_BUFFER_3D_W = 1280.0f;			//フレームバッファの幅(3D描画)
	float FRAME_BUFFER_3D_H = 720.0f;			//フレームバッファの高さ(3D描画)


	bool m_useVSync = false;//垂直同期するか

	D3D_FEATURE_LEVEL		m_featureLevel;				//Direct3D デバイスのターゲットとなる機能セット。
	ID3D11Device*			m_pd3dDevice = NULL;		//D3D11デバイス。
	IDXGISwapChain*			m_pSwapChain = NULL;		//スワップチェイン。
	ID3D11DeviceContext*	m_pd3dDeviceContext = NULL;	//D3D11デバイスコンテキスト。
	ID3D11RenderTargetView* m_backBuffer = NULL;		//バックバッファ。
	ID3D11RasterizerState*	m_rasterizerState = NULL;	//ラスタライザステート。
	ID3D11Texture2D*		m_depthStencil = NULL;		//デプスステンシル。
	ID3D11DepthStencilView* m_depthStencilView = NULL;	//デプスステンシルビュー。
	ID3D11DepthStencilState* m_depthStencilState = nullptr;

	D3D11_VIEWPORT m_viewport;

	//Sprite
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	//フルスクリーン描画プリミティブ
	CPrimitive m_fullscreen;

	//レンダー
	RanderManager m_renderManager;
	GBufferRender m_gbufferRender;
	ShadowMapRender m_shadowMapRender;
	AmbientOcclusionRender m_ambientOcclusionRender;
	DefferdRender m_defferdRender;
	MotionBlurRender m_motionBlurRender;
#ifndef DW_MASTER
	PhysicsDebugDrawRender m_physicsDebugDrawRender;
#endif
	EffekseerRender m_effekseerRender;
	std::unique_ptr<FinalRender> m_finalRender[2];
	std::unique_ptr<CameraSwitchRender> m_cameraSwitchRender[2];

	//最終レンダーターゲット
	CFinalRenderTarget m_FRT;

	//カメラマネージャー
	CameraManager m_cameraManager;

	//ライトマネージャー
	LightManager m_lightManager;

};

}