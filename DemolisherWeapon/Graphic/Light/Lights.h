#pragma once

namespace DemolisherWeapon {

//�f�B���N�V�������C�g

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

	void SetColor(const CVector3& color) {
		m_struct.color = color;
	}
	void SetDirection(const CVector3& dir) {
		m_struct.direction = dir;
		m_struct.direction.Normalize();
	}

	const SDirectionLight& GetStruct() { return m_struct; }

private:
	SDirectionLight m_struct;
};

}

//�|�C���g���C�g

struct SPointLight {
	CVector3	position;		//!<�ʒu�B
	//CVector3	positionInView;	//!<�r���[��Ԃł̍��W�B
	CVector3	color;			//!<���C�g�̃J���[�B
	//CVector4	attn;			//!<�����萔�Bx�̓|�C���g���C�g�̉e�����͂��͈́By�̓|�C���g���C�g�̌������ɉe����^���܂��B
								//!<y���傫���Ȃ�ƁA�����������Ȃ�܂��B1.0�Ő��`�̌������ɂȂ�܂��Bz,w�͖��g�p�B
	float range;
	float attenuation;
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

	const SPointLight& GetStruct() { return m_struct; }

private:
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
		CVector3 eyePos;			//�����̈ʒu�B
		int numDirectionLight;		//�f�B���N�V�������C�g�̐��B
		int numPointLight;			//�|�C���g���C�g�̐��B
		CVector3 ambientLight;		//�A���r�G���g���C�g�B
	};
	SLightParam							m_lightParam;
	ID3D11Buffer*						m_lightParamCB = nullptr;			//!<GPU�Ŏg�p���郉�C�g�p�̃p�����[�^�̒萔�o�b�t�@�B	
	
	static const int DIRLIGHT_NUM = 64;
	SDirectionLight						m_rawDirectionLights[DIRLIGHT_NUM];
	std::list<GameObj::CDirectionLight*>	m_directionLights;					//!<���s�����̃��X�g�B
	ID3D11Buffer*						m_directionLightSB = nullptr;		//!<���s�����̃��X�g�̃X�g���N�`���[�o�b�t�@�B
	ID3D11ShaderResourceView*	m_directionLightSRV = nullptr;

	static const int POILIGHT_NUM = 128;
	SPointLight							m_rawPointLights[POILIGHT_NUM];
	std::list<GameObj::CPointLight*>		m_pointLights;					//!<�|�C���g���C�g�̃��X�g�B
	ID3D11Buffer*						m_pointLightsSB = nullptr;		//!<�|�C���g���C�g�̃��X�g�̃X�g���N�`���[�o�b�t�@�B
	ID3D11ShaderResourceView*	m_pointLightsSRV = nullptr;
};

}