#pragma once

namespace DemolisherWeapon {

//ディレクションライト

struct SDirectionLight {
	CVector3 color;
	CVector3 direction;
};

namespace GameObj {

class CDirectionLight : public IGameObject {
public:
	CDirectionLight() {};
	CDirectionLight(const CVector3& color, const CVector3& dir) {
		m_struct.color = color;
		m_struct.direction = dir;
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
		m_struct.direction = dir;
		m_struct.direction.Normalize();
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

class CPointLight : public IGameObject {
public:
	CPointLight() {};
	CPointLight(const CVector3& color, const CVector3& pos, const float range, const float attenuation) {
		m_struct.color = color;
		m_pos = pos;
		m_struct.range = range;
		m_struct.attenuation = attenuation;
		m_struct.attenuation = max(0.001f, m_struct.attenuation);
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

	void UpdateBuffers();

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

private:
	struct SLightParam {
		CVector3 eyePos;			//視線の位置。
		int numDirectionLight;		//ディレクションライトの数。
		int numPointLight;			//ポイントライトの数。
		CVector3 ambientLight;		//アンビエントライト。
	};
	SLightParam							m_lightParam;
	ID3D11Buffer*						m_lightParamCB = nullptr;			//!<GPUで使用するライト用のパラメータの定数バッファ。	
	
	static const int DIRLIGHT_NUM = 64;
	SDirectionLight						m_rawDirectionLights[DIRLIGHT_NUM];
	std::list<GameObj::CDirectionLight*>	m_directionLights;					//!<平行光源のリスト。
	ID3D11Buffer*						m_directionLightSB = nullptr;		//!<平行光源のリストのストラクチャーバッファ。
	ID3D11ShaderResourceView*	m_directionLightSRV = nullptr;

	static const int POILIGHT_NUM = 128;
	SPointLight							m_rawPointLights[POILIGHT_NUM];
	std::list<GameObj::CPointLight*>		m_pointLights;					//!<ポイントライトのリスト。
	ID3D11Buffer*						m_pointLightsSB = nullptr;		//!<ポイントライトのリストのストラクチャーバッファ。
	ID3D11ShaderResourceView*	m_pointLightsSRV = nullptr;
};

}