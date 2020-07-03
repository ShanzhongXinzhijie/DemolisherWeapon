#pragma once

namespace DemolisherWeapon {

//�f�B���N�V�������C�g

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

	//�L���E������ݒ�
	void SetActiveFlag(bool active) {
		m_active = active;
	};

	//�p�����[�^�[�ݒ�
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

//�|�C���g���C�g

struct SPointLight {
	CVector3	position;		//!<�ʒu�B
	//CVector3	positionInView;	//!<�r���[��Ԃł̍��W�B
	CVector3	color;			//!<���C�g�̃J���[�B
	//CVector4	attn;			//!<�����萔�Bx�̓|�C���g���C�g�̉e�����͂��͈́By�̓|�C���g���C�g�̌������ɉe����^���܂��B
								//!<y���傫���Ȃ�ƁA�����������Ȃ�܂��B1.0�Ő��`�̌������ɂȂ�܂��Bz,w�͖��g�p�B
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

	//�L���E������ݒ�
	void SetActiveFlag(bool active) {
		m_active = active;
	};

	//�p�����[�^�[�ݒ�
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

	void UpdateStructuredBuffers();//�X�g���N�`���[�o�b�t�@���X�V
	void UpdateConstantBuffer();//�萔�o�b�t�@���X�V

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

	//�t�H�O�̗L��������ݒ�
	void SetEnableFog(bool enable) {
		m_lightParam.fogEnable = enable ? 1 : 0;
	}
	bool GetEnableFog()const {
		return m_lightParam.fogEnable;
	}
	//�t�H�O�����S�ɂ����鋗����ݒ�
	void SetFogDistance(float distance) {
		m_lightParam.fogFar = distance;
	}
	//�����t�H�O�̂�������ݒ�
	void SetFogHeightScale(float scale) {
		m_lightParam.fogHeightScale = scale;
	}
	//�t�H�O�̐F��ݒ�
	void SetFogColor(const CVector3& color) {
		m_lightParam.fogColor = color;
	}

private:
	bool m_isInit = false;

	//[defferd.fx : lightCb]
	struct SLightParam {
		CVector3 eyePos;			//�����̈ʒu�B
		int numDirectionLight;		//�f�B���N�V�������C�g�̐��B
		int numPointLight;			//�|�C���g���C�g�̐��B
		CVector3 ambientLight;		//�A���r�G���g���C�g�B

		//�t�H�O
		CVector3 fogColor = { 0.34f, 0.5f, 0.73f };	//�t�H�O�̐F
		float fogFar = 15000.0f;					//�t�H�O�����S�ɂ����鋗��
		CVector3 fogLightDir;						//���z���̌���
		float fogHeightScale = 1.5f;				//�����t�H�O�̂�����
		CVector3 fogLightColor;						//���z���̐F
		int fogEnable = 0;							//�L���t���O
	};
	SLightParam							m_lightParam;
	ID3D11Buffer*						m_lightParamCB = nullptr;			//!<GPU�Ŏg�p���郉�C�g�p�̃p�����[�^�̒萔�o�b�t�@�B	
	
	static constexpr int DIRLIGHT_NUM = 4;
	SDirectionLight						m_rawDirectionLights[DIRLIGHT_NUM];
	std::list<GameObj::CDirectionLight*>	m_directionLights;					//!<���s�����̃��X�g�B
	ID3D11Buffer*						m_directionLightSB = nullptr;		//!<���s�����̃��X�g�̃X�g���N�`���[�o�b�t�@�B
	ID3D11ShaderResourceView*	m_directionLightSRV = nullptr;

	static constexpr int POILIGHT_NUM = 12;
	SPointLight							m_rawPointLights[POILIGHT_NUM];
	std::list<GameObj::CPointLight*>		m_pointLights;					//!<�|�C���g���C�g�̃��X�g�B
	ID3D11Buffer*						m_pointLightsSB = nullptr;		//!<�|�C���g���C�g�̃��X�g�̃X�g���N�`���[�o�b�t�@�B
	ID3D11ShaderResourceView*	m_pointLightsSRV = nullptr;
};

}