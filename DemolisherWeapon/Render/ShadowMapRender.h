#pragma once
#include "IRander.h"
#include "Graphic/Shader/Shader.h"
#include "Camera/CameraManager.h"

namespace DemolisherWeapon {

class ShadowMapRender :
	public IRander
{
public:
	ShadowMapRender();
	~ShadowMapRender();

	void Init();
	void Release();

	void Render()override;

	void AddDrawModel(SkinModel* caster) {
		m_drawModelList.push_back(caster);
	}

	int GetSHADOWMAP_NUM()const{
		return SHADOWMAP_NUM;
	}
	ID3D11ShaderResourceView*& GetShadowMapSRV(int num) {
		return m_shadowMapSRV[num][0];
	}
	//ライト視点のビュープロジェクション行列を出す
	CMatrix GetLightViewProjMatrix(int num)const{
		CMatrix remat;
		remat.Mul(m_lightCam[num].GetViewMatrix(), m_lightCam[num].GetProjMatrix());
		return remat;
	}
	//ライト方向を出す
	CVector3 GetLightDir(int num)const {
		return (m_lightCam[num].GetTarget() - m_lightCam[num].GetPos()).GetNorm();
	}

	static const int SHADOWMAP_NUM = 1;

private:
	std::list<SkinModel*> m_drawModelList;

	GameObj::NoRegisterOrthoCamera m_lightCam[SHADOWMAP_NUM];//ライト視点カメラ
	ID3D11Texture2D*		m_shadowMapTex[SHADOWMAP_NUM][2] = { nullptr };		//シャドウマップテクスチャ
	ID3D11RenderTargetView* m_shadowMapView[SHADOWMAP_NUM][2] = { nullptr };		//シャドウマップビュー
	ID3D11ShaderResourceView* m_shadowMapSRV[SHADOWMAP_NUM][2] = { nullptr };		//シャドウマップSRV

	ID3D11Texture2D*		m_depthStencilTex = nullptr;				//デプスステンシルテクスチャ
	ID3D11DepthStencilView* m_depthStencilView = nullptr;				//デプスステンシルビュー

	D3D11_VIEWPORT m_viewport;//ビューポート

	ID3D11DepthStencilState* m_depthStencilState = nullptr;//デプスステンシルステート
	ID3D11RasterizerState* m_rasterizerState = nullptr;

	//ブラー関係
	Shader m_vsBlur;
	Shader m_psBlurX, m_psBlurY;
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11BlendState*	m_blendstate_NonAlpha = nullptr;

	struct ShadowBlurConstantBuffer {
		float weight[8];
	};
	ID3D11Buffer* m_sbcb = nullptr;
};

}