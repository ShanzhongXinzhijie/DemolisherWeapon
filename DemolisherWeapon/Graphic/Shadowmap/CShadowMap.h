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
	static const int SHADOWMAP_NUM = 12;

	static const int MAX_WIDTH  = 4096;
	static const int MAX_HEIGHT = 4096;

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

	//シャドウマップへの書き込みを行う前の処理
	//ビューポート・レンダーターゲット・カメラを変更する
	void PreparePreDraw();

	//シャドウマップのSRV取得
	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_resource.GetShadowMapSRV();
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

private:
	bool m_isInit = false;
	bool m_enable = true;
	bool m_enablePCSS = true;

	static ShadowMapResource m_resource;
	//static bool m_usedIndexs[ShadowMapResource::SHADOWMAP_NUM];

	GameObj::NoRegisterOrthoCamera m_lightCam;//ライト視点カメラ
	ID3D11RenderTargetView* m_shadowMapRTV = nullptr;
	//UINT m_useIndex = -1;

	D3D11_VIEWPORT m_viewport;//ビューポート

	float m_zBias = 0.0f;//深度バイアス
	// 0.00025f*0.1f; // *0.38f; // *4.0f;
};

}