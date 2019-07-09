#pragma once
#include "Camera/CameraManager.h"

namespace DemolisherWeapon {

class ShadowMapResource {
public:
	~ShadowMapResource();
	
	void Init();
	void Release();

	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_shadowMapSRV;
	}
	ID3D11Texture2D* GetShadowMapTex() {
		return m_shadowMapTex;
	}

	ID3D11DepthStencilView* GetDSV() {
		return m_depthStencilView;
	}

	//シャドウマップの最大数
	static constexpr int SHADOWMAP_NUM = DemolisherWeapon::SHADOWMAP_NUM;

	static constexpr int MAX_WIDTH  = SHADOW_MAX_WIDTH;
	static constexpr int MAX_HEIGHT = SHADOW_MAX_HEIGHT;

private:
	bool m_isInit = false;

	ID3D11Texture2D*		m_shadowMapTex = nullptr;		//シャドウマップテクスチャ
	ID3D11ShaderResourceView* m_shadowMapSRV = nullptr;		//シャドウマップSRV

	ID3D11Texture2D*		m_depthStencilTex = nullptr;	//デプスステンシルテクスチャ
	ID3D11DepthStencilView* m_depthStencilView = nullptr;	//デプスステンシルビュー
};

class CShadowMap
{
public:
	~CShadowMap();

	//シャドウマップを作成
	void Init(UINT width, UINT height, UINT index);

	//シャドウマップを削除
	void Release();

	bool GetIsInit()const { return m_isInit; }
	bool GetEnable()const { return m_isInit && m_enable; }
	bool GetEnablePCSS()const {	return m_enablePCSS; }
	
	bool GetIsRenderAndUpdateInterval() {
		m_isRenderCnt--; if (m_isRenderCnt < 0) { m_isRenderCnt = 0; }
		bool Out = m_isRenderCnt ? false : true;
		if (m_isRenderCnt == 0 || m_isRenderCnt > m_renderInterval) { m_isRenderCnt = m_renderInterval; }
		return Out;
	}

	//シャドウマップを更新していいか取得
	bool GetIsUpdate()const { return m_isUpdate; }

	float GetSizeX()const { return m_viewport.Width; }
	float GetSizeY()const { return m_viewport.Height; }

	//シャドウマップをクリア
	void ClearShadowMap();

	//シャドウマップへの書き込みを行う前の処理
	//ビューポート・レンダーターゲット・カメラを変更する
	void PreparePreDraw();

	//シャドウマップのSRV取得
	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_resource.GetShadowMapSRV();
	}
	//DSV取得
	static ID3D11DepthStencilView* GetShadowMapDSV() {
		return m_resource.GetDSV();
	}
	//RTV取得
	ID3D11RenderTargetView*& GetShadowMapRTV() {
		return m_shadowMapRTV;
	}

	//ライト視点のビュープロジェクション行列を出す
	CMatrix GetLightViewProjMatrix()const {
		CMatrix remat;
		remat.Mul(m_lightCam.GetViewMatrix(), m_lightCam.GetProjMatrix());
		return remat;
	}
	//ライト方向を出す
	CVector3 GetLightDir()const {
		return (m_lightCam.GetTarget() - m_lightCam.GetPos()).GetNorm();
	}
	//深度バイアスを取得
	float GetDepthBias()const {
		return m_zBias;
	}

	//カスケードの範囲取得
	float GetCascadeNear()const {
		return m_cascadeAreaNear;
	}
	float GetCascadeFar()const {
		return m_cascadeAreaFar;
	}


	//カメラを取得
	GameObj::NoRegisterOrthoCamera& GetLightCamera() {
		return m_lightCam;
	}

	//有効・無効を設定
	void SetEnable(bool enable) {
		m_enable = enable;
	}
	//PCSSの有効・無効を設定
	void SetEnablePCSS(bool enable) {
		m_enablePCSS = enable;
	}

	//深度バイアスを設定
	void SetDepthBias(float bias) {
		m_zBias = bias;
	}

	//カスケードの範囲設定
	void SetCascadeNear(float Near) {
		m_cascadeAreaNear = Near;
	}
	void SetCascadeFar(float Far) {
		m_cascadeAreaFar = Far;
	}

	//更新間隔を設定
	void SetRenderInterval(int interval) {
		m_renderInterval = interval;
	}

	//シャドウを更新するか設定
	void SetIsUpdate(bool isUpdate) {
		m_isUpdate = isUpdate;
	}

private:
	bool m_isInit = false;
	bool m_enable = true;
	bool m_enablePCSS = true;
	int  m_isRenderCnt = 0, m_renderInterval = 0;
	bool m_isUpdate = true;

	static ShadowMapResource m_resource;
	//static bool m_usedIndexs[ShadowMapResource::SHADOWMAP_NUM];

	GameObj::NoRegisterOrthoCamera m_lightCam;//ライト視点カメラ
	ID3D11RenderTargetView* m_shadowMapRTV = nullptr;
	//UINT m_useIndex = -1;

	D3D11_VIEWPORT m_viewport;//ビューポート

	float m_zBias = 0.0f;//深度バイアス
	// 0.00025f*0.1f; // *0.38f; // *4.0f;

	//カスケード用パラルータ
	float m_cascadeAreaNear = 0.0f, m_cascadeAreaFar = 1.0f;
};

}