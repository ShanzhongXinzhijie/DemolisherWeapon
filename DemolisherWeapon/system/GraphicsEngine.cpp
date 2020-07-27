#include "DWstdafx.h"
#include "GraphicsEngine.h"
#include "GraphicsAPI/DirectX12/DX12Test.h"
#include "GraphicsAPI/DirectX11/DX11Test.h"

namespace DemolisherWeapon {

namespace {
	constexpr int oneloopOffset = 5000;
}

GraphicsEngine::GraphicsEngine(): m_primitiveRender2D(&m_primitiveRender)
{
	
}
GraphicsEngine::~GraphicsEngine()
{
	Release();
}

void GraphicsEngine::ClearBackBuffer()
{
	m_graphicsAPI->ClearBackBuffer();
}
void GraphicsEngine::SetBackBufferToRenderTarget()
{
	m_graphicsAPI->SetBackBufferToRenderTarget();
}
void GraphicsEngine::SwapBackBuffer()
{
	m_graphicsAPI->SwapBackBuffer();
}
void GraphicsEngine::ExecuteCommand() {
	m_graphicsAPI->ExecuteCommand();
}
void GraphicsEngine::RunRenderManager() {
	m_renderManager.Render();
}

void GraphicsEngine::Release() {
	m_FRT.Release();
	
	m_useAPI = enNum;
	m_graphicsAPI.reset();
	m_dx11 = nullptr;
	m_dx12 = nullptr;
}

bool GraphicsEngine::Init(HWND hWnd, const InitEngineParameter& initParam, GameObjectManager* gom, CFpsCounter* fc) {
	//�t���[���o�b�t�@�T�C�Y
	FRAME_BUFFER_W = (float)initParam.frameBufferWidth;
	FRAME_BUFFER_H = (float)initParam.frameBufferHeight;

	//��ʕ����p�̔䗦��
	FRAME_BUFFER_3D_W = (float)initParam.frameBufferWidth3D;
	FRAME_BUFFER_3D_H = (float)initParam.frameBufferHeight3D;
	m_isSplitScreen = initParam.isSplitScreen;
	if (initParam.isSplitScreen == enVertical_TwoSplit) {
		FRAME_BUFFER_3D_H *= 0.5f;
	}
	if (initParam.isSplitScreen == enSide_TwoSplit) {
		FRAME_BUFFER_3D_W *= 0.5f;
	}

	//���������ݒ�
	m_useVSync = initParam.useVSync;

	//�����_�[�N���X�ɎQ�Ɠn��
	m_directxtkRender.Init(gom, fc);

	//�O���t�B�b�N�XAPI���Ƃ̏�����
#ifdef DW_DX12
	return InnerInitDX12(hWnd, initParam);
#else
	return InnerInitDX11(hWnd, initParam);
#endif	
}

#ifdef DW_DX12
bool GraphicsEngine::InnerInitDX12(HWND hWnd, const InitEngineParameter& initParam) {
	//DirectX12������
	m_useAPI = enDirectX12;
	m_graphicsAPI = std::make_unique<DX12Test>();
	auto sucsses = m_graphicsAPI->Init(hWnd, initParam);
	if (!sucsses) {
		return false;
	}
	m_dx12 = dynamic_cast<DX12Test*>(m_graphicsAPI.get());

	//sprite������
	{
		m_commonStates = std::make_unique<DirectX::CommonStates>(m_dx12->GetD3D12Device());

		DirectX::ResourceUploadBatch resourceUpload(m_dx12->GetD3D12Device());

		//�f�B�X�N���v�^�q�[�v���
		m_xtk12_resourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(m_dx12->GetD3D12Device(), Descriptors::Count);
		// �R�}���h�L���[���쐬
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		auto hr = m_dx12->GetD3D12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_xtk12_commandQueue));
		if (FAILED(hr)) {
			return false;
		}
		//�Ȃ񂩍��
		m_xtk12_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_dx12->GetD3D12Device());

		//�R�}���h���X�g���
		resourceUpload.Begin();
			
		//�t�H���g�쐬
		m_spriteFont = std::make_unique<DirectX::SpriteFont>(m_dx12->GetD3D12Device(), resourceUpload,
			L"Preset/Font/myfile.spritefont",
			m_xtk12_resourceDescriptors->GetCpuHandle(Descriptors::MyFont),
			m_xtk12_resourceDescriptors->GetGpuHandle(Descriptors::MyFont));		

		//�X�v���C�g�o�b�`�쐬		
		DirectX::RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);//�����_�[�^�[�Q�b�g�̏�񂪂���(�o�b�N�o�b�t�@�̂��̂��g�p)
		{//alphaBlend
			//�p�C�v���C���X�e�[�g(�u�����h�X�e�[�g�Ƃ��V�F�[�_�Ƃ��ݒ肷��)
			DirectX::SpriteBatchPipelineStateDescription pd(rtState);
			//��Ԃ̑g�ݍ��킹���Ƃ�SpriteBatch(Pipeline State Object<PSO>)���K�v
			m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_dx12->GetD3D12Device(), resourceUpload, pd);			
		}
		{//PMABlend
			//�p�C�v���C���X�e�[�g(�u�����h�X�e�[�g�Ƃ��V�F�[�_�Ƃ��ݒ肷��)
			DirectX::SpriteBatchPipelineStateDescription pd(rtState, &DirectX::CommonStates::NonPremultiplied);
			//��Ԃ̑g�ݍ��킹���Ƃ�SpriteBatch(Pipeline State Object<PSO>)���K�v
			m_spriteBatchPMA = std::make_unique<DirectX::SpriteBatch>(m_dx12->GetD3D12Device(), resourceUpload, pd);			
		}

		//�R�}���h���X�g���L���[�֑���
		auto uploadResourcesFinished = resourceUpload.End(m_xtk12_commandQueue.Get());

		//�����I���҂�
		uploadResourcesFinished.wait();
	}

	//�����_�[�̓o�^
	m_dx12Render.Init(m_dx12);
	m_renderManager.AddRender(-2, &m_dx12Render);
	
	/*
	//�����������_�[
	m_renderManager.AddRender(-3, &m_initRender);

	int screencnt = m_isSplitScreen ? 2 : 1;
	int offset = oneloopOffset * (screencnt + 1);
	//2dinit
	m_renderManager.AddRender(offset + 1, &m_initRender2D);//�r���[�|�[�g�ݒ�R�}���h���X�g��
	//primrender2D
	//m_renderManager.AddRender(offset + 2, &m_primitiveRender2D);

	//DirectXTKRender
	m_renderManager.AddRender(offset + 3, &m_directxtkRender);

	//finishrender
	m_renderManager.AddRender(offset + 4, &m_SUSRTFinishRender);	
	*/

	return true;
}
#endif

bool GraphicsEngine::InnerInitDX11(HWND hWnd, const InitEngineParameter& initParam) {	
	//DirectX11������
	m_useAPI = enDirectX11;
	m_graphicsAPI = std::make_unique<DX11Test>();
	auto sucsses = m_graphicsAPI->Init(hWnd, initParam);
	if (!sucsses) {
		return false;
	}
	m_dx11 = dynamic_cast<DX11Test*>(m_graphicsAPI.get());

	//�V�F�[�_�[�̃p�X�ݒ�(�f�o�b�O�p)
	ShaderResources::GetInstance().SetIsReplaceForEngineFilePath(initParam.isShaderPathReplaceForEngineFilePath);
	//�V�F�[�_�[�̍ăR���p�C���ݒ�
	ShaderResources::GetInstance().SetIsRecompile(initParam.isShaderRecompile);

	//�r���[�|�[�g���������B
	SetViewport(0.0f, 0.0f, FRAME_BUFFER_W, FRAME_BUFFER_H);

#ifdef DW_DX11
	//�u�����h�X�e�[�g������
	m_commonStates = std::make_unique<DirectX::CommonStates>(GetD3DDevice());
	GetD3DDeviceContext()->OMSetBlendState(m_commonStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);

	//Sprite������
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(GetD3DDevice(), L"Preset/Font/myfile.spritefont");
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(GetD3DDeviceContext());
	m_spriteBatchPMA = std::make_unique<DirectX::SpriteBatch>(GetD3DDeviceContext());

	//���[�h��ʕ`��
	{
		//�w�i�F
		//float ClearColor[4] = { 0.0f,0.3f,0.95f,1.0f };
		//m_pd3dDeviceContext->ClearRenderTargetView(m_backBuffer, ClearColor);
		//m_pd3dDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//�`�����o�b�N�o�b�t�@�ɂ���
		SetBackBufferToRenderTarget();
		
		//2D�p�̐ݒ�ɂ���
		SetViewport(0.0f, 0.0f, GetFrameBuffer_W(), GetFrameBuffer_H());

		//�e�L�X�g
		GetSpriteBatch()->Begin();// DirectX::SpriteSortMode::SpriteSortMode_Deferred, GetGraphicsEngine().GetCommonStates().NonPremultiplied());
		GetSpriteFont()->DrawString(
			GetEngine().GetGraphicsEngine().GetSpriteBatch(),
			L"�s���s�O�v�t�q�l�v�����s��\n"
			L"����N�����͎��Ԃ�������܂�...(������)",
			{ 0.5f * GetFrameBuffer_W(), 0.5f * GetFrameBuffer_H() },
			{ 1.0f,1.0f,1.0f,1.0f }, 0.0f, DirectX::XMFLOAT2(0.5f, 0.5f), 0.5f
		);
		GetSpriteBatch()->End();

		//�o�b�N�o�b�t�@��\��
		SwapBackBuffer();
	}
#endif

	//�t���X�N���[���`��v���~�e�B�u������	
	m_fullscreen.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, m_vertex, 4, m_index);

	//���C�g�}�l�[�W���[
	m_lightManager.Init();

	//�ŏI�����_�[�^�[�Q�b�g
	m_FRT.Init();

	//�����_�[������
	m_gbufferRender.Init();
	m_shadowMapRender.SetSetting(initParam.shadowMapSetting);
	m_postDrawModelRender.Init();
	m_ambientOcclusionRender.Init(initParam.SSAOBufferScale);	 m_ambientOcclusionRender.SetEnable(initParam.isEnableSSAO);
	m_defferdRender.Init();
	m_bloomRender.Init(initParam.bloomBufferScale);				 m_bloomRender.SetEnable(initParam.isEnableBloom);
	m_DOFRender.Init(initParam.DOFBufferScale);					 m_DOFRender.SetEnable(initParam.isEnableDOF);
	m_motionBlurRender.Init();									 m_motionBlurRender.SetEnable(initParam.isEnableMotionBlur);
	m_ConvertLinearToSRGB.Init();
	m_primitiveRender.Init();

	if (initParam.isSplitScreen) {
		//��ʕ���������

		//�ŏI�����_�[
		m_finalRender[0] = std::make_unique<FinalRender>();
		m_finalRender[1] = std::make_unique<FinalRender>();
		if (initParam.isSplitScreen == enVertical_TwoSplit) {
			m_finalRender[1]->Init({ 0.0f,0.0f }, { 1.0f,0.5f });
			m_finalRender[0]->Init({ 0.0f,0.5f }, { 1.0f,1.0f });
		}
		if (initParam.isSplitScreen == enSide_TwoSplit) {
			m_finalRender[0]->Init({ 0.0f,0.0f }, { 0.5f,1.0f });
			m_finalRender[1]->Init({ 0.5f,0.0f }, { 1.0f,1.0f });
		}

		//�`��O���������_�[
		m_preRenderRender[0] = std::make_unique<PreRenderRender>();
		m_preRenderRender[1] = std::make_unique<PreRenderRender>();
		m_preRenderRender[0]->Init(0);
		m_preRenderRender[1]->Init(1);

		//HUD�`�惌���_�[
		for (int i = 0; i < 2; i++) {
			CVector2 areaMin, areaMax;
			m_finalRender[i]->GetDrawArea(areaMin, areaMax);
			m_HUDRender[i] = std::make_unique<HUDRender>();
			m_HUDRender[i]->Init(i, areaMin, areaMax, { (float)initParam.HUDWidth, (float)initParam.HUDHeight });
		}

		//�J�����؂�ւ������_�[
		m_cameraSwitchRender[0] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[1] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[0]->Init(0);
		m_cameraSwitchRender[1]->Init(1);
	}
	else {
		//�P��ʏ�����

		//�ŏI�����_�[
		m_finalRender[0] = std::make_unique<FinalRender>();
		m_finalRender[0]->Init();

		//�`��O���������_�[
		m_preRenderRender[0] = std::make_unique<PreRenderRender>();
		m_preRenderRender[0]->Init(0);

		//HUD�`�惌���_�[
		for (int i = 0; i < 1; i++) {
			CVector2 areaMin, areaMax;
			m_finalRender[i]->GetDrawArea(areaMin, areaMax);
			m_HUDRender[i] = std::make_unique<HUDRender>();
			m_HUDRender[i]->Init(i, areaMin, areaMax, { (float)initParam.HUDWidth, (float)initParam.HUDHeight });
		}
	}
	FinalRender::SetIsLensDistortion(initParam.isLensDistortion);
	FinalRender::SetIsAntiAliasing(initParam.isAntiAliasing);

	//�����_�[���Z�b�g	

	//�����������_�[
	m_renderManager.AddRender(-3, &m_initRender);
	//�V���h�E�}�b�v�`��
	m_renderManager.AddRender(-2, &m_shadowMapRender);

	//��ʕ����������s
	int screencnt = m_isSplitScreen ? 2 : 1;
	for (int i = 0; i < screencnt; i++) {

		int offset = oneloopOffset * i;		

		if (initParam.isSplitScreen) {
			//��ʕ����Ȃ�J�����؂�ւ�
			m_renderManager.AddRender(-1 + offset, m_cameraSwitchRender[i].get());			
		}

		//�`��O����
		m_renderManager.AddRender(0 + offset, m_preRenderRender[i].get());
		
		//G�o�b�t�@�`��
		m_renderManager.AddRender(1 + offset, &m_gbufferRender);

		//AO�}�b�v�쐬
		m_renderManager.AddRender(3 + offset, &m_ambientOcclusionRender);

		//�f�B�t�@�[�h�����_�����O
		m_renderManager.AddRender(4 + offset, &m_defferdRender);

		//�|�X�g�v���Z�X
		m_renderManager.AddRender(5 + offset, &m_DOFRender);
		m_renderManager.AddRender(6 + offset, &m_motionBlurRender);
		m_renderManager.AddRender(7 + offset, &m_bloomRender);

		//�|�X�g�h���[���f��
		m_renderManager.AddRender(8 + offset, &m_postDrawModelRender);

		//SRGB�ɕϊ�
		m_renderManager.AddRender(9 + offset, &m_ConvertLinearToSRGB);

		//Effekseer�̕`��
		m_renderManager.AddRender(10 + offset, &m_effekseerRender);
		
		m_freeRenderPriority = 11;//�������火�܂Ŗ��g�p

		//�v���~�e�B�u�`��
		m_renderManager.AddRender(998 + offset, &m_primitiveRender);

#ifndef DW_MASTER
		//BUlletPhysics�̃f�o�b�O�`��
		m_renderManager.AddRender(999 + offset, &m_physicsDebugDrawRender);
#endif
		//�ŏI�`��
		m_renderManager.AddRender(1000 + offset, m_finalRender[i].get());	

		//HUD�`��
		m_renderManager.AddRender(1001 + offset, m_HUDRender[i].get());
	}	
	int offset = oneloopOffset * (screencnt + 1);
	//2dinit
	m_renderManager.AddRender(offset + 1, &m_initRender2D);
	//primrender2D
	m_renderManager.AddRender(offset + 2, &m_primitiveRender2D);
	//DirectXTKRender
	m_renderManager.AddRender(offset + 3, &m_directxtkRender);
	//finishrender
	m_renderManager.AddRender(offset + 4, &m_SUSRTFinishRender);

	//GPU�C�x���g�p
#ifndef DW_MASTER
	m_dx11->GetD3DDeviceContext()->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_pUserAnotation);
#endif

	return true;
}

void GraphicsEngine::ChangeFrameBufferSize(int frameBufferWidth, int frameBufferHeight, int frameBufferWidth3D, int frameBufferHeight3D, int HUDWidth, int HUDHeight, EnSplitScreenMode screenMode, float* splitScreenSize) {
	//�T�C�Y�ύX
	FRAME_BUFFER_W = (float)frameBufferWidth;
	FRAME_BUFFER_H = (float)frameBufferHeight;
	
	//�O���t�B�b�N�XApi���Ƃ̏���
	m_graphicsAPI->ChangeFrameBufferSize();

	//�r���[�|�[�g���������B
	SetViewport(0.0f, 0.0f, FRAME_BUFFER_W, FRAME_BUFFER_H);

	//��ʕ����ɕύX���邩?
	bool isChangeSplitScreen = false;
	if (m_isSplitScreen == enNoSplit && screenMode != enNoSplit || m_isSplitScreen != enNoSplit && screenMode == enNoSplit) {
		isChangeSplitScreen = true;
	}
	m_isSplitScreen = screenMode;

	//��ʕ����p�̔䗦��
	FRAME_BUFFER_3D_W = (float)frameBufferWidth3D;
	FRAME_BUFFER_3D_H = (float)frameBufferHeight3D;
	
	//�ŏI�����_�[�^�[�Q�b�g�ď�����
	m_FRT.Init();

	//�����_�[���T�C�Y
	m_gbufferRender.Resize();
	m_postDrawModelRender.Resize();
	m_ambientOcclusionRender.Resize();
	m_defferdRender.Resize();
	m_bloomRender.Resize();	
	m_DOFRender.Resize();	
	m_motionBlurRender.Resize();	
	m_ConvertLinearToSRGB.Resize();
	m_primitiveRender.Resize();		
	//HUD���T�C�Y
	for (auto& hud : m_HUDRender) {
		if (hud) {
			hud->Resize({ (float)HUDWidth, (float)HUDHeight });
		}
	}

	//��ʕ����ύX
	if (isChangeSplitScreen) {		
		//�Ċm��
		m_finalRender[0].reset();
		m_finalRender[1].reset();
		m_preRenderRender[0].reset();
		m_preRenderRender[1].reset();
		m_cameraSwitchRender[0].reset();
		m_cameraSwitchRender[1].reset();
		m_HUDRender[0].reset();
		m_HUDRender[1].reset();

		if (m_isSplitScreen) {
			//��ʕ�����������

			//�ŏI�����_�[�̊m��
			m_finalRender[0] = std::make_unique<FinalRender>();
			m_finalRender[1] = std::make_unique<FinalRender>();
			//�X�N���[���T�C�Y�ݒ�
			if (splitScreenSize) {
				m_finalRender[0]->Init({ splitScreenSize[0],splitScreenSize[1] }, { splitScreenSize[2],splitScreenSize[3] });
				m_finalRender[1]->Init({ splitScreenSize[4],splitScreenSize[5] }, { splitScreenSize[6],splitScreenSize[7] });
			}
			else {
				if (m_isSplitScreen == enVertical_TwoSplit) {
					m_finalRender[1]->Init({ 0.0f,0.0f }, { 1.0f,0.5f });
					m_finalRender[0]->Init({ 0.0f,0.5f }, { 1.0f,1.0f });
				}
				if (m_isSplitScreen == enSide_TwoSplit) {
					m_finalRender[0]->Init({ 0.0f,0.0f }, { 0.5f,1.0f });
					m_finalRender[1]->Init({ 0.5f,0.0f }, { 1.0f,1.0f });
				}
			}

			//�`��O���������_�[
			m_preRenderRender[0] = std::make_unique<PreRenderRender>();
			m_preRenderRender[1] = std::make_unique<PreRenderRender>();
			m_preRenderRender[0]->Init(0);
			m_preRenderRender[1]->Init(1);			

			//�J�����؂�ւ������_�[�̏�����
			m_cameraSwitchRender[0] = std::make_unique<CameraSwitchRender>();
			m_cameraSwitchRender[1] = std::make_unique<CameraSwitchRender>();
			m_cameraSwitchRender[0]->Init(0);
			m_cameraSwitchRender[1]->Init(1);
		}
		else {
			//�P��ʎ�������

			//�ŏI�����_�[�̏�����
			m_finalRender[0] = std::make_unique<FinalRender>();
			m_finalRender[0]->Init();

			//�`��O���������_�[
			m_preRenderRender[0] = std::make_unique<PreRenderRender>();
			m_preRenderRender[0]->Init(0);
		}

		//HUD�����_�[
		for (int i = 0; i < (m_isSplitScreen ? 2 : 1); i++) {
			CVector2 areaMin, areaMax;
			m_finalRender[i]->GetDrawArea(areaMin, areaMax);
			m_HUDRender[i] = std::make_unique<HUDRender>();
			m_HUDRender[i]->Init(i, areaMin, areaMax, { (float)HUDWidth, (float)HUDHeight });
		}

		//��ʕ����������s
		int screencnt = 2;// m_isSplitScreen ? 2 : 1;
		for (int i = 0; i < screencnt; i++) {

			int offset = oneloopOffset * i;

			//�Ċm�ۑg�̍ēo�^
			{
				//�����_�[���X�g����폜
				m_renderManager.DeleteRender(-1 + offset);
				m_renderManager.DeleteRender(0 + offset);
				m_renderManager.DeleteRender(1000 + offset);
				m_renderManager.DeleteRender(1001 + offset);

				//�o�^
				if (m_isSplitScreen) {
					//�J�����؂�ւ�
					m_renderManager.AddRender(-1 + offset, m_cameraSwitchRender[i].get());
				}
				if (m_isSplitScreen || i == 0) {
					//�`��O����
					m_renderManager.AddRender(0 + offset, m_preRenderRender[i].get());
					//�ŏI�`��
					m_renderManager.AddRender(1000 + offset, m_finalRender[i].get());	
					//HUD�`��
					m_renderManager.AddRender(1001 + offset, m_HUDRender[i].get());
				}				
			}

			if (i == 0) { 
				continue;//���̐�͈�T�ڂ͎��s���Ȃ�
			}

			if (m_isSplitScreen) {
				//�ǉ�

				//G�o�b�t�@�`��
				m_renderManager.AddRender(1 + offset, &m_gbufferRender);

				//AO�}�b�v�쐬
				m_renderManager.AddRender(3 + offset, &m_ambientOcclusionRender);

				//�f�B�t�@�[�h�����_�����O
				m_renderManager.AddRender(4 + offset, &m_defferdRender);

				//�|�X�g�v���Z�X
				m_renderManager.AddRender(5 + offset, &m_DOFRender);
				m_renderManager.AddRender(6 + offset, &m_motionBlurRender);
				m_renderManager.AddRender(7 + offset, &m_bloomRender);

				//�|�X�g�h���[���f��
				m_renderManager.AddRender(8 + offset, &m_postDrawModelRender);

				//SRGB�ɕϊ�
				m_renderManager.AddRender(9 + offset, &m_ConvertLinearToSRGB);

				//Effekseer�̕`��
				m_renderManager.AddRender(10 + offset, &m_effekseerRender);

				//m_freeRenderPriority = 11;//�������火�܂Ŗ��g�p

				//�v���~�e�B�u�`��
				m_renderManager.AddRender(998 + offset, &m_primitiveRender);

#ifndef DW_MASTER
				//BUlletPhysics�̃f�o�b�O�`��
				m_renderManager.AddRender(999 + offset, &m_physicsDebugDrawRender);
#endif
			}
			else {
				//�폜

				//G�o�b�t�@�`��
				m_renderManager.DeleteRender(1 + offset);

				//AO�}�b�v�쐬
				m_renderManager.DeleteRender(3 + offset);

				//�f�B�t�@�[�h�����_�����O
				m_renderManager.DeleteRender(4 + offset);

				//�|�X�g�v���Z�X
				m_renderManager.DeleteRender(5 + offset);
				m_renderManager.DeleteRender(6 + offset);
				m_renderManager.DeleteRender(7 + offset);

				//�|�X�g�h���[���f��
				m_renderManager.DeleteRender(8 + offset);

				//SRGB�ɕϊ�
				m_renderManager.DeleteRender(9 + offset);

				//Effekseer�̕`��
				m_renderManager.DeleteRender(10 + offset);

				//m_freeRenderPriority = 11;//�������火�܂Ŗ��g�p

				//�v���~�e�B�u�`��
				m_renderManager.DeleteRender(998 + offset);

#ifndef DW_MASTER
				//BUlletPhysics�̃f�o�b�O�`��
				m_renderManager.DeleteRender(999 + offset);
#endif
			}
		}
	}
}

//�`�����ŏI�����_�[�^�[�Q�b�g�ɂ���
void GraphicsEngine::SetFinalRenderTarget() {
	GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), m_FRT.GetDSV());
}
//�f�v�X���g�p��
void GraphicsEngine::SetFinalRenderTarget_NonDepth() {
	GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(1, &m_FRT.GetRTV(), nullptr);
}

}