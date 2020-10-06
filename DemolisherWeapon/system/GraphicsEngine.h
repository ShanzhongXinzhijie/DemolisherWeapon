#pragma once

#include"Render/IRander.h"
#include"Render/GBufferRender.h"
#include"Render/ShadowMapRender.h"
#include"Render/AmbientOcclusionRender.h"
#include"Render/DefferdRender.h"
#include"Render/FinalRender.h"
#include"Render/BloomRender.h"
#include"Render/DepthOfFieldRender.h"
#include"Render/MotionBlurRender.h"
#include"Render/PostDrawModelRender.h"
#include"Render/PhysicsDebugDrawRender.h"
#include"Render/CameraSwitchRender.h"
#include"Render/EffekseerRender.h"
#include"Render/ConvertLinearToSRGBRender.h"
#include"Render/PrimitiveRender.h"
#include"Render/InitRender.h"
#include"Render/PreRenderRender.h"
#include"Render/HUDRender.h"
#include"Render/DirectXTKRender.h"
#include"Render/finishrender.h"

#include"GraphicsAPI/DirectX12/DescriptorHeapXTK12.h"
#include"GraphicsAPI/DirectX12/DX12Test.h"
#include"GraphicsAPI/DirectX11/DX11Test.h"
#include"Render/DX12Render.h"

#include"Camera/CameraManager.h"
#include"Graphic/Light/Lights.h"
#include"Graphic/CPrimitive.h"

#include"Graphic/Model/ModelDrawMode.h"

namespace DemolisherWeapon {

struct InitEngineParameter;

enum EnSplitScreenMode {
	enNoSplit = 0,
	enVertical_TwoSplit,
	enSide_TwoSplit,
};

enum EnGraphicsAPI {
	enDirectX11,
	enDirectX12,
	enNum,
};

class GraphicsEngine
{
public:
	GraphicsEngine();
	~GraphicsEngine();

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="hWnd">�E�B���h�E�n���h��</param>
	/// <param name="initParam">�������p�����[�^</param>
	bool Init(HWND hWnd, const InitEngineParameter& initParam, GameObjectManager*, CFpsCounter*);

	/// <summary>
	/// �J��
	/// </summary>
	void Release();

	/// <summary>
	/// �g�p���Ă���O���t�B�b�N�XAPI�̎�ނ��擾
	/// </summary>
	EnGraphicsAPI GetUseAPI()const {
		return m_useAPI;
	}

	/// <summary>
	/// D3D11�f�o�C�X���擾
	/// </summary>
	ID3D11Device* GetD3DDevice()
	{
		return m_dx11->GetD3DDevice();
	}
	/// <summary>
	/// D3D11�f�o�C�X�R���e�L�X�g���擾
	/// </summary>
	ID3D11DeviceContext* GetD3DDeviceContext()
	{
		return m_dx11->GetD3DDeviceContext();
	}
	
	/// <summary>
	/// D3D12�f�o�C�X���擾
	/// </summary>
	ID3D12Device* GetD3D12Device()
	{
		return m_dx12->GetD3D12Device();
	}

	/// <summary>
	/// �R�}���h���X�g���擾
	/// </summary>
	ID3D12GraphicsCommandList* GetCommandList()
	{
		return m_dx12->GetCommandList();
	}
	/*
	/// <summary>
	/// �R�}���h�L���[���擾
	/// </summary>
	ID3D12CommandQueue* GetCommandQueue()
	{
		return m_dx12->GetCommandQueue();
	}
	*/

	/// <summary>
	/// DirectX12�N���X���擾
	/// </summary>
	DX12Test& GetDX12() {
		return *m_dx12;
	}

#ifdef DW_DX12
	/// <summary>
	/// DirectXTK12�̃f�B�X�N���v�^�q�[�v���擾
	/// </summary>
	auto GetDirectXTK12DescriptorHeap() {
		return m_xtk12_resourceDescriptors.Heap();
	}
	int CreateDirectXTK12DescriptorNumber(D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {
		return m_xtk12_resourceDescriptors.CreateDescriptorNumber(cpuHandle, gpuHandle);
	}
	/// <summary>
	/// DirectXTK12�p�R�}���h�L���[���擾
	/// </summary>
	ID3D12CommandQueue* GetXTK12CommandQueue()
	{
		return m_xtk12_commandQueue.Get();
	}
#endif	

	//�t���[���o�b�t�@�T�C�Y�̕ύX
	void ChangeFrameBufferSize(	int frameBufferWidth, int frameBufferHeight,
								int frameBuffer3DWidth, int frameBuffer3DHeight,
								int HUDWidth, int HUDHeight,
								EnSplitScreenMode screenMode, float* splitScreenSize = nullptr);

	//�t���[���o�b�t�@�̎擾
	float GetFrameBuffer_W()const {
		return m_isChangeFrameBufferSize ? m_frameBufferW : FRAME_BUFFER_W;
	}
	float GetFrameBuffer_H()const {
		return m_isChangeFrameBufferSize ? m_frameBufferH : FRAME_BUFFER_H;
	}
	float Get3DFrameBuffer_W()const{
		return FRAME_BUFFER_3D_W;
	}
	float Get3DFrameBuffer_H()const {
		return FRAME_BUFFER_3D_H;
	}
	//�擾�ł���t���[���o�b�t�@�T�C�Y�̕ύX
	void ChangeFrameBufferSize(float W, float H) {
		m_isChangeFrameBufferSize = true;
		m_frameBufferW = W; m_frameBufferH = H;
	}
	//�擾�ł���t���[���o�b�t�@�T�C�Y�����Ƃɖ߂�
	void ResetFrameBufferSize(){ m_isChangeFrameBufferSize = false; }

	//��ʕ����ݒ���擾
	EnSplitScreenMode GetSplitScreenMode()const {
		return m_isSplitScreen;
	}

	//�^�[�Q�b�g�ɂȂ��Ă�X�N���[���ԍ����擾
	int GetTargetScreenNum()const {
		return m_HUDNum;
	}
	void SetTargetScreenNum(int num) {
		m_HUDNum = num;
	}

	//���������҂���ݒ�
	void SetUseVSync(bool b) {
		m_useVSync = b;
	}
	bool GetUseVSync() {
		return m_useVSync;
	}

	//���X�^���C�U�[�X�e�[�g�����Z�b�g
	void ResetRasterizerState() {
		m_dx11->ResetRasterizerState();
	}

	//�o�b�N�o�b�t�@���N���A
	void ClearBackBuffer();
	//�o�b�N�o�b�t�@�������_�[�^�[�Q�b�g�ɐݒ�
	void SetBackBufferToRenderTarget();
	//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���
	void SwapBackBuffer();

	//�R�}���h���X�g�̎��s
	void ExecuteCommand();

	//DirectXTK�̃R�}���h���X�g�̎��s
	void ExecuteCommandDirectXTK() {
#ifdef DW_DX12
		m_xtk12_graphicsMemory->Commit(m_xtk12_commandQueue.Get());
#endif
	}
	
	//�����_�[�}�l�[�W���[�̕`��
	void RunRenderManager();

	//Sprite�擾
	DirectX::SpriteBatch* GetSpriteBatch() const
	{
		return m_spriteBatch.get();
	}
	//��Z�ς݃A���t�@��
	DirectX::SpriteBatch* GetSpriteBatchPMA() const
	{
		return m_spriteBatchPMA.get();
	}
	DirectX::SpriteFont* GetSpriteFont() const
	{
		return m_spriteFont.get();
	}
	float AddAndGetLayerDepthCnt() {
		m_layerDepthCnt += 0.001f / 2048.0f;
		return m_layerDepthCnt;
	}
	void ResetLayerDepthCnt() {
		m_layerDepthCnt = 0.0f;
	}

	//�t���X�N���[���`��
	void DrawFullScreen() {
		m_fullscreen.DrawIndexed();
	}

	//3D���f�������_�[�ɕ`�悷�郂�f����o�^
	void AddDrawModelToD3Render(SkinModel* sm, int priority, bool reverse) {
		m_gbufferRender.AddDrawModel(sm, priority, reverse);
	}
	void AddDrawModelToShadowMapRender(SkinModel* sm, int priority, bool reverse) {
		m_shadowMapRender.AddDrawModel(sm, priority, reverse);
	}
	void AddDrawModelToPostDrawRender(SkinModel* sm, int priority, PostDrawModelRender::enBlendMode blendmode, bool reverse = false) {
		m_postDrawModelRender.AddDrawModel(sm, priority, blendmode, reverse);
	}	

	//���f���`�惂�[�h�̎擾
	ModelDrawMode& GetModelDrawMode() {
		return m_modelDrawMode;
	}

	//�����_�[�}�l�[�W���[�̎擾
	RanderManager& GetRenderManager(){
		return m_renderManager;
	}
	int GetFreeRenderPriority()const{
		return m_freeRenderPriority;
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
	//�f�B�t�@�[�h�����_�[�̎擾
	DefferdRender& GetDefferdRender() {
		return m_defferdRender;
	}
	//�u���[�������_�[�̎擾
	BloomRender& GetBloomRender() {
		return m_bloomRender;
	}
	//��ʊE�[�x�����_�[�̎擾
	DepthOfFieldRender& GetDOFRender() {
		return m_DOFRender;
	}
	//���[�V�����u���[�����_�[�̎擾
	MotionBlurRender& GetMotionBlurRender() {
		return m_motionBlurRender;
	}
	//�v���~�e�B�u�����_�[�̎擾
	PrimitiveRender& GetPrimitiveRender() {
		return m_primitiveRender;
	}
	//�t�@�C�i�������_�[�̎擾
	FinalRender& GetFinalRender(int index) {
		return *m_finalRender[index];
	}

	//PhysicsDebugDrawRender�̃��[�h�ݒ�	
	void SetPhysicsDebugDrawMode(int debugMode) {
#ifndef DW_MASTER	
		m_physicsDebugDrawRender.SetDebugMode(debugMode);
#endif
	}
	//PhysicsDebugDrawRender���L�����擾
	bool GetEnablePhysicsDebugDraw() {
#ifndef DW_MASTER	
		return m_physicsDebugDrawRender.IsEnable();
#endif
		return false;
	}

	//�J�����}�l�[�W���[�擾
	CameraManager& GetCameraManager(){
		return m_cameraManager;
	}

	//���C�g�}�l�[�W���[�擾
	LightManager& GetLightManager() {
		return m_lightManager;
	}

	//�`�����ŏI�����_�[�^�[�Q�b�g��
	void SetFinalRenderTarget();
	//�f�v�X���g�p��
	void SetFinalRenderTarget_NonDepth();
	//�ŏI�����_�[�^�[�Q�b�g�擾
	CFinalRenderTarget& GetFRT() { return m_FRT; }

	//�r���[�|�[�g�ݒ�
	void SetViewport(float topLeftX, float topLeftY, float width, float height)
	{
		m_graphicsAPI->SetViewport(topLeftX, topLeftY, width, height);
	}

	/// <summary>
	/// �R�����X�e�[�g�̎擾
	/// </summary>
	const DirectX::CommonStates& GetCommonStates()const {
		return *m_commonStates.get();
	}

	/// <summary>
	/// GPU�C�x���g�̊J�n
	/// </summary>
	/// <param name="eventName">�C�x���g��</param>
	void BeginGPUEvent(const wchar_t* eventName)
	{
#ifndef DW_MASTER
		if (m_pUserAnotation) {
			m_pUserAnotation->BeginEvent(eventName);
		}
#endif
	}
	/// <summary>
	/// GPU�C�x���g�̏I��
	/// </summary>
	void EndGPUEvent()
	{
#ifndef DW_MASTER
		if (m_pUserAnotation) {
			m_pUserAnotation->EndEvent();
		}
#endif
	}

private:
	bool InnerInitDX11(HWND hWnd, const InitEngineParameter& initParam);
#ifdef DW_DX12
	bool InnerInitDX12(HWND hWnd, const InitEngineParameter& initParam);
#endif

private:
	float FRAME_BUFFER_W = 1280.0f;				//�t���[���o�b�t�@�̕��B
	float FRAME_BUFFER_H = 720.0f;				//�t���[���o�b�t�@�̍����B
	float FRAME_BUFFER_3D_W = 1280.0f;			//�t���[���o�b�t�@�̕�(3D�`��)
	float FRAME_BUFFER_3D_H = 720.0f;			//�t���[���o�b�t�@�̍���(3D�`��)
	
	//�擾����t���[���o�b�t�@�T�C�Y
	bool m_isChangeFrameBufferSize = false;
	float m_frameBufferW = FRAME_BUFFER_W;
	float m_frameBufferH = FRAME_BUFFER_H;

	EnSplitScreenMode m_isSplitScreen = enNoSplit;//��ʕ����ݒ�

	bool m_useVSync = false;//�����������邩
	
	EnGraphicsAPI m_useAPI = enNum;
	std::unique_ptr<IGraphicsAPI> m_graphicsAPI;//�O���t�B�b�N�XAPI
	DX11Test* m_dx11 = nullptr;
	DX12Test* m_dx12 = nullptr;

	std::unique_ptr<DirectX::CommonStates> m_commonStates;//�R�����X�e�[�g

	//Sprite
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatchPMA;
	float m_layerDepthCnt = 0.0f;

#ifdef DW_DX12
	//Directx12
	DX12Render m_dx12Render;

	//DirectXTK12	
	DescriptorHeapXTK12 m_xtk12_resourceDescriptors;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_xtk12_commandQueue;
	std::unique_ptr<DirectX::GraphicsMemory> m_xtk12_graphicsMemory;
#endif

	//�t���X�N���[���`��v���~�e�B�u
	CPrimitive m_fullscreen;
	SVertex m_vertex[4] = {
		{
			{-1.0f, -1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f}
		},
		{
			{1.0f, -1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f}
		},
		{
			{-1.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f}
		},
		{
			{1.0f, 1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f}
		},
	};
	unsigned long m_index[4] = { 0,1,2,3 };

	//���f���`�惂�[�h
	ModelDrawMode m_modelDrawMode;

	//�����_�[
	int m_freeRenderPriority = -1;
	RanderManager m_renderManager;
	InitRender m_initRender;
	GBufferRender m_gbufferRender;
	ShadowMapRender m_shadowMapRender;
	AmbientOcclusionRender m_ambientOcclusionRender;
	DefferdRender m_defferdRender;
	BloomRender m_bloomRender;
	DepthOfFieldRender m_DOFRender;
	MotionBlurRender m_motionBlurRender;
	PostDrawModelRender m_postDrawModelRender;
	ConvertLinearToSRGBRender m_ConvertLinearToSRGB;
	PrimitiveRender m_primitiveRender;
#ifndef DW_MASTER
	PhysicsDebugDrawRender m_physicsDebugDrawRender;
#endif
	EffekseerRender m_effekseerRender;
	std::unique_ptr<FinalRender> m_finalRender[2];
	std::unique_ptr<CameraSwitchRender> m_cameraSwitchRender[2];
	std::unique_ptr<PreRenderRender> m_preRenderRender[2];
	std::unique_ptr<HUDRender> m_HUDRender[2];
	InitRender2D m_initRender2D;
	PrimitiveRender2D m_primitiveRender2D;
	DirectXTKRender m_directxtkRender;
	SuperUltraSeriousRealTrueFinishRender m_SUSRTFinishRender;

	//���݃^�[�Q�b�g�ɂȂ��Ă����ʔԍ�
	int m_HUDNum = -1;

	//�ŏI�����_�[�^�[�Q�b�g
	CFinalRenderTarget m_FRT;

	//�J�����}�l�[�W���[
	CameraManager m_cameraManager;

	//���C�g�}�l�[�W���[
	LightManager m_lightManager;

	//GPU�C�x���g�p
#ifndef DW_MASTER
	ID3DUserDefinedAnnotation* m_pUserAnotation = nullptr;
#endif
};

}