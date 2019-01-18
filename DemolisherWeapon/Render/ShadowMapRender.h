#pragma once
#include "IRander.h"
#include "Graphic/Shader/Shader.h"
#include "Graphic/Shadowmap/CShadowMap.h"

namespace DemolisherWeapon {

enum EnShadowMapMode {
	enOFF,
	enON,
	enPCSS,
};

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

	//シャドウマップ全体の有効・無効を設定
	void SetSetting(EnShadowMapMode setting) {
		m_setting = setting;
	}

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
		return (m_setting != enOFF) && m_shadowMaps[num].GetEnable();
	}
	//PCSSが有効か取得
	bool GetEnablePCSS(int num)const {
		return m_setting == enPCSS && m_shadowMaps[num].GetEnablePCSS();
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
	//深度バイアスを取得
	float GetDepthBias(int num)const {
		return  m_shadowMaps[num].GetDepthBias();
	}

	//シャドウマップの最大数
	static const int SHADOWMAP_NUM = 12;

private:
	EnShadowMapMode m_setting = enPCSS;

	std::list<SkinModel*> m_drawModelList;

	CShadowMap m_shadowMaps[SHADOWMAP_NUM];
};

}