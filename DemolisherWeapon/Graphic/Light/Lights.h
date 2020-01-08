#pragma once

namespace DemolisherWeapon {

//ディレクションライト

struct SDirectionLight {
	CVector3 color;
	CVector3 direction;
};

namespace GameObj {

class CDirectionLight : public IQSGameObject {
public:
	CDirectionLight() {};
	CDirectionLight(const CVector3& color, const CVector3& dir) {
		m_struct.color = color;
		m_struct.direction = dir * -1.0f;
		m_struct.direction.Normalize();
	};

	void PostLoopUpdate()override;

	//有効・無効を設定
	void SetActiveFlag(bool active) {
		m_active = active;
	};

	//パラメーター設定
	void SetColor(const CVector3& color) {
		m_struct.color = color;
	}
	void SetDirection(const CVector3& dir) {
		m_struct.direction = dir * -1.0f;
		m_struct.direction.Normalize();
	}

	const CVector3& GetColor() const {
		return m_struct.color;
	}
	CVector3 GetDirection() const {
		return m_struct.direction * -1.0f;
	}

	const SDirectionLight& GetStruct() const{ return m_struct; }
	bool GetActiveFlag() const { return m_active; }

private:
	bool m_active = true;
	SDirectionLight m_struct;
};

}

//ポイントライト

struct SPointLight {
	CVector3	position;		//!<位置。
	//CVector3	positionInView;	//!<ビュー空間での座標。
	CVector3	color;			//!<ライトのカラー。
	//CVector4	attn;			//!<減衰定数。xはポイントライトの影響が届く範囲。yはポイントライトの減衰率に影響を与えます。
								//!<yが大きくなると、減衰が強くなります。1.0で線形の減衰率になります。z,wは未使用。
	float range = 0.0f;
	float attenuation = 1.0f;
};

namespace GameObj {

class CPointLight : public IQSGameObject {
public:
	CPointLight() {};
	CPointLight(const CVector3& color, const CVector3& pos, const float range, const float attenuation) {
		m_struct.color = color;
		m_pos = pos;
		m_struct.range = range;
		m_struct.attenuation = attenuation;
		m_struct.attenuation = max(0.001f, m_struct.attenuation);
	};

	void PostLoopUpdate()override final;

	//有効・無効を設定
	void SetActiveFlag(bool active) {
		m_active = active;
	};

	//パラメーター設定
	void SetColor(const CVector3& color) {
		m_struct.color = color;
	}
	void SetPos(const CVector3& pos) {
		m_pos = pos;
	}
	void SetRange(const float range) {
		m_struct.range = range;
	}
	void SetAttenuation(const float attenuation) {
		m_struct.attenuation = attenuation;
		m_struct.attenuation = max(0.001f, m_struct.attenuation);
	}


	const SPointLight& GetStruct() const{ return m_struct; }
	bool GetActiveFlag() const { return m_active; }

private:
	bool m_active = true;
	SPointLight m_struct;
	CVector3 m_pos;
};

}

class LightManager
{
public:
	~LightManager();

	void Init();
	void Release();

	void UpdateStructuredBuffers();//ストラクチャーバッファを更新
	void UpdateConstantBuffer();//定数バッファを更新

	ID3D11Buffer*& GetLlightParamCB()	{ return m_lightParamCB; }
	ID3D11ShaderResourceView*& GetDirectionLightSRV() { return m_directionLightSRV; }
	ID3D11ShaderResourceView*& GetPointLightsSRV()	{ return m_pointLightsSRV; }
	
	void SetAmbientLight(const CVector3& ambient){
		m_lightParam.ambientLight = ambient;
	}

	void AddDirectionLight(GameObj::CDirectionLight* dl) {
		m_directionLights.emplace_back(dl);
	}
	void AddPointLight(GameObj::CPointLight* pl) {
		m_pointLights.emplace_back(pl);
	}

	//フォグの有効無効を設定
	void SetEnableFog(bool enable) {
		m_lightParam.fogEnable = enable ? 1 : 0;
	}
	bool GetEnableFog()const {
		return m_lightParam.fogEnable;
	}
	//フォグが完全にかかる距離を設定
	void SetFogDistance(float distance) {
		m_lightParam.fogFar = distance;
	}
	//高さフォグのかかり具合を設定
	void SetFogHeightScale(float scale) {
		m_lightParam.fogHeightScale = scale;
	}
	//フォグの色を設定
	void SetFogColor(const CVector3& color) {
		m_lightParam.fogColor = color;
	}

private:
	//[defferd.fx : lightCb]
	struct SLightParam {
		CVector3 eyePos;			//視線の位置。
		int numDirectionLight;		//ディレクションライトの数。
		int numPointLight;			//ポイントライトの数。
		CVector3 ambientLight;		//アンビエントライト。

		//フォグ
		CVector3 fogColor = { 0.34f, 0.5f, 0.73f };	//フォグの色
		float fogFar = 15000.0f;					//フォグが完全にかかる距離
		CVector3 fogLightDir;						//太陽光の向き
		float fogHeightScale = 1.5f;				//高さフォグのかかり具合
		CVector3 fogLightColor;						//太陽光の色
		int fogEnable = 0;							//有効フラグ
	};
	SLightParam							m_lightParam;
	ID3D11Buffer*						m_lightParamCB = nullptr;			//!<GPUで使用するライト用のパラメータの定数バッファ。	
	
	static constexpr int DIRLIGHT_NUM = 4;
	SDirectionLight						m_rawDirectionLights[DIRLIGHT_NUM];
	std::list<GameObj::CDirectionLight*>	m_directionLights;					//!<平行光源のリスト。
	ID3D11Buffer*						m_directionLightSB = nullptr;		//!<平行光源のリストのストラクチャーバッファ。
	ID3D11ShaderResourceView*	m_directionLightSRV = nullptr;

	static constexpr int POILIGHT_NUM = 12;
	SPointLight							m_rawPointLights[POILIGHT_NUM];
	std::list<GameObj::CPointLight*>		m_pointLights;					//!<ポイントライトのリスト。
	ID3D11Buffer*						m_pointLightsSB = nullptr;		//!<ポイントライトのリストのストラクチャーバッファ。
	ID3D11ShaderResourceView*	m_pointLightsSRV = nullptr;
};

}