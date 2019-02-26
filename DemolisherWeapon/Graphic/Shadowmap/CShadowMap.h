#pragma once
#include "Camera/CameraManager.h"

namespace DemolisherWeapon {

class ShadowMapResource {
public:
	~ShadowMapResource();
	
	void Init();
	void Release();

	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_shadowMapSRV;
	}
	ID3D11Texture2D* GetShadowMapTex() {
		return m_shadowMapTex;
	}

	ID3D11DepthStencilView* GetDSV() {
		return m_depthStencilView;
	}

	//�V���h�E�}�b�v�̍ő吔
	static const int SHADOWMAP_NUM = DemolisherWeapon::SHADOWMAP_NUM;

	static const int MAX_WIDTH  = SHADOW_MAX_WIDTH;
	static const int MAX_HEIGHT = SHADOW_MAX_HEIGHT;

private:
	bool m_isInit = false;

	ID3D11Texture2D*		m_shadowMapTex = nullptr;		//�V���h�E�}�b�v�e�N�X�`��
	ID3D11ShaderResourceView* m_shadowMapSRV = nullptr;		//�V���h�E�}�b�vSRV

	ID3D11Texture2D*		m_depthStencilTex = nullptr;	//�f�v�X�X�e���V���e�N�X�`��
	ID3D11DepthStencilView* m_depthStencilView = nullptr;	//�f�v�X�X�e���V���r���[
};

class CShadowMap
{
public:
	~CShadowMap();

	//�V���h�E�}�b�v���쐬
	void Init(UINT width, UINT height, UINT index);

	//�V���h�E�}�b�v���폜
	void Release();

	bool GetIsInit()const { return m_isInit; }
	bool GetEnable()const { return m_isInit && m_enable; }
	bool GetEnablePCSS()const {	return m_enablePCSS; }
	
	bool GetIsRenderAndUpdateInterval() {
		m_isRenderCnt--; if (m_isRenderCnt < 0) { m_isRenderCnt = 0; }
		bool Out = m_isRenderCnt ? false : true;
		if (m_isRenderCnt == 0 || m_isRenderCnt > m_renderInterval) { m_isRenderCnt = m_renderInterval; }
		return Out;
	}

	float GetSizeX()const { return m_viewport.Width; }
	float GetSizeY()const { return m_viewport.Height; }

	//�V���h�E�}�b�v���N���A
	void ClearShadowMap();

	//�V���h�E�}�b�v�ւ̏������݂��s���O�̏���
	//�r���[�|�[�g�E�����_�[�^�[�Q�b�g�E�J������ύX����
	void PreparePreDraw();

	//�V���h�E�}�b�v��SRV�擾
	ID3D11ShaderResourceView*& GetShadowMapSRV() {
		return m_resource.GetShadowMapSRV();
	}
	//DSV�擾
	static ID3D11DepthStencilView* GetShadowMapDSV() {
		return m_resource.GetDSV();
	}
	//RTV�擾
	ID3D11RenderTargetView*& GetShadowMapRTV() {
		return m_shadowMapRTV;
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
	//�[�x�o�C�A�X���擾
	float GetDepthBias()const {
		return m_zBias;
	}

	//�J�X�P�[�h�͈͎̔擾
	float GetCascadeNear()const {
		return m_cascadeAreaNear;
	}
	float GetCascadeFar()const {
		return m_cascadeAreaFar;
	}


	//�J�������擾
	GameObj::NoRegisterOrthoCamera& GetLightCamera() {
		return m_lightCam;
	}

	//�L���E������ݒ�
	void SetEnable(bool enable) {
		m_enable = enable;
	}
	//PCSS�̗L���E������ݒ�
	void SetEnablePCSS(bool enable) {
		m_enablePCSS = enable;
	}

	//�[�x�o�C�A�X��ݒ�
	void SetDepthBias(float bias) {
		m_zBias = bias;
	}

	//�J�X�P�[�h�͈̔͐ݒ�
	void SetCascadeNear(float Near) {
		m_cascadeAreaNear = Near;
	}
	void SetCascadeFar(float Far) {
		m_cascadeAreaFar = Far;
	}

	//�X�V�Ԋu��ݒ�
	void SetRenderInterval(int interval) {
		m_renderInterval = interval;
	}

private:
	bool m_isInit = false;
	bool m_enable = true;
	bool m_enablePCSS = true;
	int m_isRenderCnt = 0, m_renderInterval = 0;

	static ShadowMapResource m_resource;
	//static bool m_usedIndexs[ShadowMapResource::SHADOWMAP_NUM];

	GameObj::NoRegisterOrthoCamera m_lightCam;//���C�g���_�J����
	ID3D11RenderTargetView* m_shadowMapRTV = nullptr;
	//UINT m_useIndex = -1;

	D3D11_VIEWPORT m_viewport;//�r���[�|�[�g

	float m_zBias = 0.0f;//�[�x�o�C�A�X
	// 0.00025f*0.1f; // *0.38f; // *4.0f;

	//�J�X�P�[�h�p�p�����[�^
	float m_cascadeAreaNear = 0.0f, m_cascadeAreaFar = 1.0f;
};

}