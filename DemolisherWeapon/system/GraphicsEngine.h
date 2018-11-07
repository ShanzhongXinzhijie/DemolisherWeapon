#pragma once

#include"Render/IRander.h"
#include"Render/GBufferRender.h"
#include"Render/ShadowMapRender.h"
#include"Render/AmbientOcclusionRender.h"
#include"Render/DefferdRender.h"
#include"Render/FinalRender.h"
#include"Render/MotionBlurRender.h"
#include"Render/PhysicsDebugDrawRender.h"

#include"Camera/CameraManager.h"
#include"Graphic/Light/Lights.h"
#include"Graphic/CPrimitive.h"

namespace DemolisherWeapon {

class GraphicsEngine
{
public:
	GraphicsEngine();
	~GraphicsEngine();

	/*!
	 *@brief	�������B
	 *@param[in]	hWnd		�E�B���h�E�n���h���B
	 */
	void Init(HWND hWnd, int bufferW, int bufferH, int refreshRate, bool isWindowMode);
	/*!
	 *@brief	����B
	 */
	void Release();
	/*!
	 *@brief	D3D11�f�o�C�X���擾�B
	 */
	ID3D11Device* GetD3DDevice()
	{
		return m_pd3dDevice;
	}
	/*!
	 *@brief	D3D11�f�o�C�X�R���e�L�X�g���擾�B
	 */
	ID3D11DeviceContext* GetD3DDeviceContext()
	{
		return m_pd3dDeviceContext;
	}

	//�t���[���o�b�t�@�̎擾
	float GetFrameBuffer_W()const{
		return FRAME_BUFFER_W;
	}
	float GetFrameBuffer_H()const {
		return FRAME_BUFFER_H;
	}

	//���������҂���ݒ�
	void SetUseVSync(bool b) {
		m_useVSync = b;
	}

	//���X�^���C�U�[�X�e�[�g�����Z�b�g
	void ResetRasterizerState() {
		m_pd3dDeviceContext->RSSetState(m_rasterizerState);
	}

	//�o�b�N�o�b�t�@���N���A���ĕ`���ɐݒ�
	void ResetBackBuffer();
	//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���
	void SwapBackBuffer();
	
	//�����_�[�}�l�[�W���[�̕`��
	void RunRenderManager();

	//Sprite�擾
	DirectX::SpriteBatch* GetSpriteBatch() const
	{
		return m_spriteBatch.get();
	}
	DirectX::SpriteFont* GetSpriteFont() const
	{
		return m_spriteFont.get();
	}

	//�t���X�N���[���`��
	void DrawFullScreen() {
		m_fullscreen.DrawIndexed();
	}

	//3D���f�������_�[�ɕ`�悷�郂�f����o�^
	void AddDrawModelToD3Render(SkinModel* sm) {
		m_gbufferRender.AddDrawModel(sm);//m_d3render.AddDrawModel(sm);
	};
	void AddDrawModelToShadowMapRender(SkinModel* sm) {
		m_shadowMapRender.AddDrawModel(sm);
	}

	//G�o�b�t�@�����_�[�̎擾
	GBufferRender& GetGBufferRender() {
		return m_gbufferRender;
	}
	//�V���h�E�}�b�v�����_�[�̎擾
	ShadowMapRender& GetShadowMapRender() {
		return m_shadowMapRender;
	}
	//�A���r�G���g�I�N���[�W���������_�[�̎擾
	AmbientOcclusionRender& GetAmbientOcclusionRender() {
		return m_ambientOcclusionRender;
	}
	//PhysicsDebugDrawRender�̎擾
	//PhysicsDebugDrawRender& GetPhysicsDebugDrawRender() {
	//	return m_physicsDebugDrawRender;
	//}
	void SetPhysicsDebugDrawMode(int debugMode) {
#ifdef _DEBUG	
		m_physicsDebugDrawRender.SetDebugMode(debugMode);
#endif
	}
	//�t�@�C�i�������_�[�̎擾
	FinalRender& GetFinalRender() {
		return m_finalRender;
	}

	//�J�����}�l�[�W���[�擾
	CameraManager& GetCameraManager(){
		return m_cameraManager;
	}

	//���C�g�}�l�[�W���[�擾
	LightManager& GetLightManager() {
		return m_lightManager;
	}

private:

	float FRAME_BUFFER_W = 1280.0f;				//�t���[���o�b�t�@�̕��B
	float FRAME_BUFFER_H = 720.0f;				//�t���[���o�b�t�@�̍����B

	bool m_useVSync = false;//�����������邩

	D3D_FEATURE_LEVEL		m_featureLevel;				//Direct3D �f�o�C�X�̃^�[�Q�b�g�ƂȂ�@�\�Z�b�g�B
	ID3D11Device*			m_pd3dDevice = NULL;		//D3D11�f�o�C�X�B
	IDXGISwapChain*			m_pSwapChain = NULL;		//�X���b�v�`�F�C���B
	ID3D11DeviceContext*	m_pd3dDeviceContext = NULL;	//D3D11�f�o�C�X�R���e�L�X�g�B
	ID3D11RenderTargetView* m_backBuffer = NULL;		//�o�b�N�o�b�t�@�B
	ID3D11RasterizerState*	m_rasterizerState = NULL;	//���X�^���C�U�X�e�[�g�B
	ID3D11Texture2D*		m_depthStencil = NULL;		//�f�v�X�X�e���V���B
	ID3D11DepthStencilView* m_depthStencilView = NULL;	//�f�v�X�X�e���V���r���[�B
	ID3D11DepthStencilState* m_depthStencilState = nullptr;

	//Sprite
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	//�t���X�N���[���`��v���~�e�B�u
	CPrimitive m_fullscreen;

	//�����_�[
	RanderManager m_renderManager;
	GBufferRender m_gbufferRender;
	ShadowMapRender m_shadowMapRender;
	AmbientOcclusionRender m_ambientOcclusionRender;
	DefferdRender m_defferdRender;
	MotionBlurRender m_motionBlurRender;
#ifdef _DEBUG
	PhysicsDebugDrawRender m_physicsDebugDrawRender;
#endif
	FinalRender m_finalRender;

	//�J�����}�l�[�W���[
	CameraManager m_cameraManager;

	//���C�g�}�l�[�W���[
	LightManager m_lightManager;

};

}