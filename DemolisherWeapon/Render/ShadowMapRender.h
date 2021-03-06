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
	ShadowMapRender() = default;
	~ShadowMapRender();

	void Render()override;
	void PostRender()override;

	void Resize()override {};

public:
	//描画前後に行う処理を設定するためのクラス
	/*class IPrePost {
	public:
		virtual ~IPrePost() {};
	public:
		virtual void PreDraw() {};		//すべてのシャドウマップの描画前に行う処理
		virtual void PreModelDraw() {};	//モデル達の描画前に行う処理
		virtual void PostModelDraw() {};//モデル達の描画後に行う処理
		virtual void PostDraw() {};		//すべてのシャドウマップの描画後に行う処理
	};*/

public:
	//開放
	void Release();

	//シャドウマップ全体の有効・無効を設定
	void SetSetting(EnShadowMapMode setting) {
		m_setting = setting;
	}

	//シャドウマップに描画するモデルを追加
	void AddDrawModel(SkinModel* caster, int priority, bool reverse) {
		m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX)].push_back({ caster, reverse });
	}
	//描画前後に行う処理を追加
	//void AddPrePostAction(IPrePost* action) {
	//	m_prePostActionList.push_back(action);
	//}

	//シャドウマップをひとつ有効化
	//UINT width, UINT height ...シャドウマップのサイズ
	CShadowMap* EnableShadowMap(UINT width, UINT height) {
		int i = 0;
		for (i = 0; i < SHADOWMAP_NUM; i++) {
			if (!m_shadowMaps[i].GetIsInit()) {
				m_shadowMaps[i].Init(width, height, i);
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
	//シャドウマップのサイズ取得
	float GetSizeX(int num)const {
		return m_shadowMaps[num].GetSizeX();
	}
	float GetSizeY(int num)const {
		return m_shadowMaps[num].GetSizeY();
	}

	//シャドウマップのSRV取得
	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_shadowMaps[0].GetShadowMapSRV();
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

	//カスケードの範囲取得
	float GetCascadeNear(int num)const {
		return m_shadowMaps[num].GetCascadeNear();
	}
	float GetCascadeFar(int num)const {
		return m_shadowMaps[num].GetCascadeFar();
	}

	//カメラを取得
	GameObj::NoRegisterOrthoCamera& GetLightCamera(int num){
		return m_shadowMaps[num].GetLightCamera();
	}

	//シャドウマップの最大数
	static constexpr int SHADOWMAP_NUM = ShadowMapResource::SHADOWMAP_NUM;

private:
	EnShadowMapMode m_setting = enPCSS;

	std::list<std::pair<SkinModel*,bool>> m_drawModelList[DRAW_PRIORITY_MAX];
	//std::list<IPrePost*> m_prePostActionList;

	CShadowMap m_shadowMaps[SHADOWMAP_NUM];//シャドウマップ
};

}