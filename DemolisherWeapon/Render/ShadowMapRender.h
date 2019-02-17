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

	//�V���h�E�}�b�v�S�̗̂L���E������ݒ�
	void SetSetting(EnShadowMapMode setting) {
		m_setting = setting;
	}

	//�V���h�E�}�b�v�ɕ`�悷�郂�f����ǉ�
	void AddDrawModel(SkinModel* caster, int priority) {
		m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX)].push_back(caster);
	}

	//�V���h�E�}�b�v���ЂƂL����
	//UINT width, UINT height ...�V���h�E�}�b�v�̃T�C�Y
	CShadowMap* EnableShadowMap(UINT width, UINT height) {
		int i = 0;
		for (i = 0; i < SHADOWMAP_NUM; i++) {
			if (!m_shadowMaps[i].GetIsInit()) {
				m_shadowMaps[i].Init(width, height, i);
				return &m_shadowMaps[i];
			}
		}
		DW_ERRORBOX(i == SHADOWMAP_NUM, "ShadowMapRender::EnableShadowMap() :�V���h�E�}�b�v������ȏ���܂���");
		return nullptr;
	}

	//�V���h�E�}�b�v�L�����擾
	bool GetShadowMapEnable(int num)const {
		return (m_setting != enOFF) && m_shadowMaps[num].GetEnable();
	}
	//PCSS���L�����擾
	bool GetEnablePCSS(int num)const {
		return m_setting == enPCSS && m_shadowMaps[num].GetEnablePCSS();
	}
	//�V���h�E�}�b�v�̃T�C�Y�擾
	float GetSizeX(int num)const {
		return m_shadowMaps[num].GetSizeX();
	}
	float GetSizeY(int num)const {
		return m_shadowMaps[num].GetSizeY();
	}

	//�V���h�E�}�b�v��SRV�擾
	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_shadowMaps[0].GetShadowMapSRV();
	}

	//���C�g���_�̃r���[�v���W�F�N�V�����s����o��
	CMatrix GetLightViewProjMatrix(int num)const{
		return m_shadowMaps[num].GetLightViewProjMatrix();
	}
	//���C�g�������o��
	CVector3 GetLightDir(int num)const {
		return m_shadowMaps[num].GetLightDir();
	}
	//�[�x�o�C�A�X���擾
	float GetDepthBias(int num)const {
		return  m_shadowMaps[num].GetDepthBias();
	}

	//�J�X�P�[�h�͈͎̔擾
	float GetCascadeNear(int num)const {
		return m_shadowMaps[num].GetCascadeNear();
	}
	float GetCascadeFar(int num)const {
		return m_shadowMaps[num].GetCascadeFar();
	}

	//�V���h�E�}�b�v�̍ő吔
	static const int SHADOWMAP_NUM = ShadowMapResource::SHADOWMAP_NUM;

private:
	EnShadowMapMode m_setting = enPCSS;

	std::list<SkinModel*> m_drawModelList[DRAW_PRIORITY_MAX];

	CShadowMap m_shadowMaps[SHADOWMAP_NUM];
};

}