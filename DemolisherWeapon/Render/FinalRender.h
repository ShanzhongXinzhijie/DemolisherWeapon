#pragma once

#include"Graphic/CPrimitive.h"

namespace DemolisherWeapon {

class CFinalRenderTarget {
public:
	~CFinalRenderTarget();

	void Init();
	void Release();

	ID3D11Texture2D*			GetTex(int num = -1) { if (num == 0 || num == 1) { return m_Tex[num]; } else { return m_Tex[m_now]; } }
	ID3D11RenderTargetView*&	GetRTV(int num = -1) { if (num == 0 || num == 1) { return m_RTV[num]; } else { return m_RTV[m_now]; } }
	ID3D11ShaderResourceView*&	GetSRV(int num = -1) { if (num == 0 || num == 1) { return m_SRV[num]; } else { return m_SRV[m_now]; } }

	ID3D11DepthStencilView* GetDSV() { return m_depthStencilView; }

	//使用ぶつの入れ替え
	void Swap() {
		m_now ^= 1;
	}
	void SetNow(int n) {
		m_now = n;
	}
	
	void Copy();

	void Clear(int num = -1);
	void AllClear();

private:
	ID3D11Texture2D*			m_Tex[2] = { nullptr };//テクスチャ
	ID3D11RenderTargetView*		m_RTV[2] = { nullptr };//RTV
	ID3D11ShaderResourceView*	m_SRV[2] = { nullptr };//SRV

	ID3D11Texture2D*		m_depthStencilTex  = nullptr;//デプスステンシルテクスチャ
	ID3D11DepthStencilView* m_depthStencilView = nullptr;//デプスステンシルビュー

	int m_now = 0;
};

class FinalRender : public IRander
{
public:
	~FinalRender();

	void Init(const CVector2 screen_min = { 0.0f,0.0f }, const CVector2 screen_max = { 1.0f,1.0f }, bool isLinearSample = true);
	void Release();

	void Render()override;

	//描画範囲を取得
	void GetDrawArea(CVector2& return_min, CVector2& return_max) {
		return_min = m_screen_min;
		return_max = m_screen_max;
	}

	//歪曲収差の係数を計算
	static float Calc_k4(float fov);
	//歪曲収差をかけた2D座標を取得
	static CVector2 CalcLensDistortion(const CVector2& pos, GameObj::ICamera* cam);

	//歪曲収差の有効・無効を設定
	static void SetIsLensDistortion(bool enable) {
		m_isLensDistortion = enable;
	}
	//歪曲収差の度合いを設定
	static void SetLensDistortionScale(float scale) {
		m_lensDistortionScale = scale;
	}
	//アンチエイリアスの有効・無効を設定
	static void SetIsAntiAliasing(bool enable) {
		m_isAntiAliasing = enable;
	}	

private:
	//シェーダーマクロ
	enum ShaderTypeMask {
		enOFF = 0b00,			//全て無効
		enLensDistortion = 0b01,
		enAntialiasing = 0b10,
		enALL = 0b11,			//全て有効
		enNum,					//全パターンの数
	};
	//マクロの数
	static constexpr int MACRO_NUM = 2;

	Shader m_vs;
	Shader m_ps[ShaderTypeMask::enNum];
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11Buffer* m_cb = nullptr;

	struct SPSConstantBuffer {
		float k4;
		float LENS_DISTORTION_UV_MAGNIFICATION;
		float ASPECT_RATIO;
		float INV_ASPECT_RATIO;

		float resolution[2];
	};

	CPrimitive m_drawSpace;
	CPrimitive::SVertex m_vertex[4];
	int m_index[4] = { 0,1,2,3 };

	//描画範囲
	CVector2 m_screen_min;
	CVector2 m_screen_max;

	ID3D11ShaderResourceView* m_gridTex = nullptr;

	//ピクセルシェーダ設定
	static bool m_isLensDistortion;
	static float m_lensDistortionScale;
	static bool m_isAntiAliasing;
};

}