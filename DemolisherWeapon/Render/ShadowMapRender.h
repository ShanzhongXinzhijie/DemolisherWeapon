#pragma once
#include "IRander.h"
#include "Graphic/Shader/Shader.h"
#include "Graphic/Shadowmap/CShadowMap.h"

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
	void PostRender()override;

	//シャドウマップに描画するモデルを追加
	void AddDrawModel(SkinModel* caster) {
		m_drawModelList.push_back(caster);
	}

	//シャドウマップをひとつ有効化
	//UINT width, UINT height ...シャドウマップのサイズ
	CShadowMap* EnableShadowMap(UINT width, UINT height) {
		int i = 0;
		for (i = 0; i < SHADOWMAP_NUM; i++) {
			if (!m_shadowMaps[i].GetIsInit()) {
				m_shadowMaps[i].Init(width, height);
				return &m_shadowMaps[i];
			}
		}
		DW_ERRORBOX(i == SHADOWMAP_NUM, "ShadowMapRender::EnableShadowMap() :シャドウマップをこれ以上作れません");
		return nullptr;
	}

	//シャドウマップ有効か取得
	bool GetShadowMapEnable(int num)const {
		return m_shadowMaps[num].GetEnable();
	}
	//シャドウマップのSRV取得
	ID3D11ShaderResourceView*& GetShadowMapSRV(int num) {
		return m_shadowMaps[num].GetShadowMapSRV();
	}
	//ライト視点のビュープロジェクション行列を出す
	CMatrix GetLightViewProjMatrix(int num)const{
		return m_shadowMaps[num].GetLightViewProjMatrix();
	}
	//ライト方向を出す
	CVector3 GetLightDir(int num)const {
		return m_shadowMaps[num].GetLightDir();
	}

	//シャドウマップの最大数
	static const int SHADOWMAP_NUM = 12;

private:
	std::list<SkinModel*> m_drawModelList;

	CShadowMap m_shadowMaps[SHADOWMAP_NUM];

	//ID3D11DepthStencilState* m_depthStencilState = nullptr;
	//ID3D11RasterizerState* m_rasterizerState = nullptr;

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