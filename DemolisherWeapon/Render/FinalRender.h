#pragma once

#include"../Graphic/CPrimitive.h"

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

	void Init(const CVector2 screen_min = { 0.0f,0.0f }, const CVector2 screen_max = { 1.0f,1.0f });
	void Release();

	void Render()override;

	//void SetFinalRenderTarget();

	//CFinalRenderTarget& GetFRT() { return m_FRT; }

private:
	//CFinalRenderTarget m_FRT;
	Shader m_vs;
	Shader m_ps;
	ID3D11SamplerState* m_samplerState = nullptr;

	CPrimitive m_drawSpace;
};

}