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

	//�V���h�E�}�b�v�ɕ`�悷�郂�f����ǉ�
	void AddDrawModel(SkinModel* caster) {
		m_drawModelList.push_back(caster);
	}

	//�V���h�E�}�b�v���ЂƂL����
	//UINT width, UINT height ...�V���h�E�}�b�v�̃T�C�Y
	CShadowMap* EnableShadowMap(UINT width, UINT height) {
		int i = 0;
		for (i = 0; i < SHADOWMAP_NUM; i++) {
			if (!m_shadowMaps[i].GetIsInit()) {
				m_shadowMaps[i].Init(width, height);
				return &m_shadowMaps[i];
			}
		}
		DW_ERRORBOX(i == SHADOWMAP_NUM, "ShadowMapRender::EnableShadowMap() :�V���h�E�}�b�v������ȏ���܂���");
		return nullptr;
	}

	//�V���h�E�}�b�v�L�����擾
	bool GetShadowMapEnable(int num)const {
		return m_shadowMaps[num].GetEnable();
	}
	//�V���h�E�}�b�v��SRV�擾
	ID3D11ShaderResourceView*& GetShadowMapSRV(int num) {
		return m_shadowMaps[num].GetShadowMapSRV();
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

	//�V���h�E�}�b�v�̍ő吔
	static const int SHADOWMAP_NUM = 12;

private:
	std::list<SkinModel*> m_drawModelList;

	CShadowMap m_shadowMaps[SHADOWMAP_NUM];
};

}