#pragma once
#include "Camera/CameraManager.h"

namespace DemolisherWeapon {

class CShadowMap
{
public:
	~CShadowMap();

	//�V���h�E�}�b�v���쐬
	void Init(UINT width, UINT height);

	//�V���h�E�}�b�v���폜
	void Release();

	bool GetIsInit()const { return m_isInit; }
	bool GetEnable()const { return m_isInit && m_enable; }

	//�V���h�E�}�b�v�ւ̏������݂��s���O�̏���
	//�r���[�|�[�g�E�����_�[�^�[�Q�b�g�E�J������ύX����
	void PreparePreDraw();

	//�V���h�E�}�b�v��SRV�擾
	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_shadowMapSRV[0];
	}
	//���C�g���_�̃r���[�v���W�F�N�V�����s����o��
	CMatrix GetLightViewProjMatrix()const {
		CMatrix remat;
		remat.Mul(m_lightCam.GetViewMatrix(), m_lightCam.GetProjMatrix());
		return remat;
	}
	//���C�g�������o��
	CVector3 GetLightDir()const {
		return (m_lightCam.GetTarget() - m_lightCam.GetPos()).GetNorm();
	}


	//�J�������擾
	GameObj::NoRegisterOrthoCamera& GetLightCamera() {
		return m_lightCam;
	}

	//�L���E������ݒ�
	void SetEnable(bool enable) {
		m_enable = enable;
	}

private:
	bool m_isInit = false;
	bool m_enable = true;

	GameObj::NoRegisterOrthoCamera m_lightCam;//���C�g���_�J����

	ID3D11Texture2D*		m_shadowMapTex[2] = { nullptr };		//�V���h�E�}�b�v�e�N�X�`��
	ID3D11RenderTargetView* m_shadowMapView[2] = { nullptr };		//�V���h�E�}�b�v�r���[
	ID3D11ShaderResourceView* m_shadowMapSRV[2] = { nullptr };		//�V���h�E�}�b�vSRV

	ID3D11Texture2D*		m_depthStencilTex = nullptr;	//�f�v�X�X�e���V���e�N�X�`��
	ID3D11DepthStencilView* m_depthStencilView = nullptr;	//�f�v�X�X�e���V���r���[

	D3D11_VIEWPORT m_viewport;//�r���[�|�[�g
};

}