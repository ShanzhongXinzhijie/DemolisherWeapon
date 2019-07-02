#include "DWstdafx.h"
#include "GraphicsEngine.h"

namespace DemolisherWeapon {

GraphicsEngine::GraphicsEngine()
{
	
}
GraphicsEngine::~GraphicsEngine()
{
	Release();
}


void GraphicsEngine::ClearBackBuffer()
{
	//�o�b�N�o�b�t�@���D�F�œh��Ԃ��B
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	m_pd3dDeviceContext->ClearRenderTargetView(m_backBuffer, ClearColor);
	m_pd3dDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);	
}
void GraphicsEngine::SetBackBufferToRenderTarget()
{
	//�`�����ݐ���o�b�N�o�b�t�@�ɂ���B
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_backBuffer, m_depthStencilView);
}
void GraphicsEngine::SwapBackBuffer()
{
	//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���B
	m_pSwapChain->Present(m_useVSync?1:0, 0);
}
void GraphicsEngine::RunRenderManager() {
	m_renderManager.Render();
}

void GraphicsEngine::Release()
{
	if (m_depthStencilState) {
		m_depthStencilState->Release();
		m_depthStencilState = nullptr;
	}
	if (m_rasterizerState != NULL) {
		m_rasterizerState->Release();
		m_rasterizerState = NULL;
	}
	if (m_depthStencil != NULL) {
		m_depthStencil->Release();
		m_depthStencil = NULL;
	}
	if (m_depthStencilView != NULL) {
		m_depthStencilView->Release();
		m_depthStencilView = NULL;
	}
	if (m_backBuffer != NULL) {
		m_backBuffer->Release();
		m_backBuffer = NULL;
	}
	if (m_pSwapChain != NULL) {
		m_pSwapChain->Release();
		m_pSwapChain = NULL;
	}
	if (m_pd3dDeviceContext != NULL) {
		m_pd3dDeviceContext->Release();
		m_pd3dDeviceContext = NULL;
	}
	if (m_pd3dDevice != NULL) {
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}

	m_FRT.Release();
}
void GraphicsEngine::Init(HWND hWnd, const InitEngineParameter& initParam)
{
	FRAME_BUFFER_W = (float)initParam.frameBufferWidth;
	FRAME_BUFFER_H = (float)initParam.frameBufferHeight;
	
	//�X���b�v�`�F�C�����쐬���邽�߂̏���ݒ肷��B
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;									//�X���b�v�`�F�C���̃o�b�t�@���B�ʏ�͂P�B
	sd.BufferDesc.Width = (UINT)FRAME_BUFFER_W;			//�t���[���o�b�t�@�̕��B
	sd.BufferDesc.Height = (UINT)FRAME_BUFFER_H;		//�t���[���o�b�t�@�̍����B
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//�t���[���o�b�t�@�̃t�H�[�}�b�g�BR8G8B8A8��32bit�B
	sd.BufferDesc.RefreshRate.Numerator = initParam.refleshRate;	//���j�^�̃��t���b�V�����[�g�B(�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���^�C�~���O�ƂȂ�B)
	sd.BufferDesc.RefreshRate.Denominator = 1;			//�Q�ɂ�����30fps�ɂȂ�B1�ł����B
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//�T�[�t�F�X�܂��̓��\�[�X���o�̓����_�[ �^�[�Q�b�g�Ƃ��Ďg�p���܂��B
	sd.OutputWindow = hWnd;								//�o�͐�̃E�B���h�E�n���h���B
	sd.SampleDesc.Count = 1;							//1�ł����B
	sd.SampleDesc.Quality = 0;							//MSAA�Ȃ��B0�ł����B
	sd.Windowed = initParam.isWindowMode ? TRUE : FALSE;			//�E�B���h�E���[�h�BTRUE�ł悢�B

	//���p����DirectX�̋@�\�Z�b�g�B
	//���̔z���D3D11CreateDeviceAndSwapChain�̈����Ƃ��Ďg���B
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		//D3D_FEATURE_LEVEL_10_1,
		//D3D_FEATURE_LEVEL_10_0,
	};

	//D3D�f�o�C�X�ƃX���b�v�`�F�C�����쐬����B
	D3D11CreateDeviceAndSwapChain(
		NULL,											//NULL�ł����B
		D3D_DRIVER_TYPE_HARDWARE,						//D3D�f�o�C�X���A�N�Z�X����h���C�o�[�̎�ށB
														//��{�I��D3D_DRIVER_TYPE_HARDWARE���w�肷��΂悢�B
		NULL,											//NULL�ł����B
		0,												//�O�ł����B
		featureLevels,									//D3D�f�o�C�X�̃^�[�Q�b�g�ƂȂ�@�\�Z�b�g���w�肷��B
		sizeof(featureLevels) / sizeof(featureLevels[0]),	//�@�\�Z�b�g�̐��B
		D3D11_SDK_VERSION,								//�g�p����DirectX�̃o�[�W�����B
														//D3D11_SDK_VERSION���w�肷��΂悢�B
		&sd,											//�X���b�v�`�F�C�����쐬���邽�߂̏��B
		&m_pSwapChain,									//�쐬�����X���b�v�`�F�C���̃A�h���X�̊i�[��B
		&m_pd3dDevice,									//�쐬����D3D�f�o�C�X�̃A�h���X�̊i�[��B
		&m_featureLevel,								//�g�p�����@�\�Z�b�g�̊i�[��B
		&m_pd3dDeviceContext							//�쐬����D3D�f�o�C�X�R���e�L�X�g�̃A�h���X�̊i�[��B
	);

	//�������ݐ�ɂȂ郌���_�����O�^�[�Q�b�g���쐬�B
	ID3D11Texture2D* pBackBuffer = NULL;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_backBuffer);
	pBackBuffer->Release();
	//�[�x�X�e���V���r���[�̍쐬�B
	{
		//�[�x�e�N�X�`���̍쐬�B
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = (UINT)FRAME_BUFFER_W;
		texDesc.Height = (UINT)FRAME_BUFFER_H;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_D32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		m_pd3dDevice->CreateTexture2D(&texDesc, NULL, &m_depthStencil);
		//�[�x�X�e���V���r���[���쐬�B
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = texDesc.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		m_pd3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
	}

	//���X�^���C�U���������B
	D3D11_RASTERIZER_DESC desc = {};
	desc.CullMode = D3D11_CULL_FRONT;
	desc.FillMode = D3D11_FILL_SOLID;
	desc.DepthClipEnable = true;
	desc.MultisampleEnable = true;
	m_pd3dDevice->CreateRasterizerState(&desc, &m_rasterizerState);
	m_pd3dDeviceContext->RSSetState(m_rasterizerState);

	//�r���[�|�[�g���������B
	SetViewport(0.0f, 0.0f, FRAME_BUFFER_W, FRAME_BUFFER_H);

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		m_pd3dDevice->CreateDepthStencilState(&desc, &m_depthStencilState);
		m_pd3dDeviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}

	//��ʕ����p�̔䗦��
	FRAME_BUFFER_3D_W = (float)initParam.frameBufferWidth3D;
	FRAME_BUFFER_3D_H = (float)initParam.frameBufferHeight3D;
	if (initParam.isSplitScreen == enVertical_TwoSplit) {
		FRAME_BUFFER_3D_H *= 0.5f;
	}
	if (initParam.isSplitScreen == enSide_TwoSplit) {
		FRAME_BUFFER_3D_W *= 0.5f;
	}

	//Sprite������
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(m_pd3dDevice, L"Preset/Font/myfile.spritefont");
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pd3dDeviceContext);
	m_spriteBatchPMA = std::make_unique<DirectX::SpriteBatch>(m_pd3dDeviceContext);

	//�t���X�N���[���`��v���~�e�B�u������
	CPrimitive::SVertex vertex[4] = {
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
	int index[4] = { 0,1,2,3 };
	m_fullscreen.Init(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, vertex, 4, index);

	//���C�g�}�l�[�W���[
	m_lightManager.Init();

	//�ŏI�����_�[�^�[�Q�b�g
	m_FRT.Init();

	//�����_�[������
	m_gbufferRender.Init();
	m_shadowMapRender.Init();									 m_shadowMapRender.SetSetting(initParam.shadowMapSetting);
	m_ambientOcclusionRender.Init(initParam.SSAOBufferScale);	 m_ambientOcclusionRender.SetEnable(initParam.isEnableSSAO);
	m_defferdRender.Init();
	m_bloomRender.Init(initParam.bloomBufferScale);				 m_bloomRender.SetEnable(initParam.isEnableBloom);
	m_DOFRender.Init(initParam.DOFBufferScale);					 m_DOFRender.SetEnable(initParam.isEnableDOF);
	m_motionBlurRender.Init();									 m_motionBlurRender.SetEnable(initParam.isEnableMotionBlur);
	m_ConvertLinearToSRGB.Init();
	m_primitiveRender.Init();

	if (initParam.isSplitScreen) {
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

		m_cameraSwitchRender[0] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[1] = std::make_unique<CameraSwitchRender>();
		m_cameraSwitchRender[0]->Init(0);
		m_cameraSwitchRender[1]->Init(1);
	}
	else {
		m_finalRender[0] = std::make_unique<FinalRender>();
		m_finalRender[0]->Init();
	}
	FinalRender::SetIsLensDistortion(initParam.isLensDistortion);

	//�����_�[���Z�b�g	

	//�V���h�E�}�b�v�`��
	m_renderManager.AddRender(-1, &m_shadowMapRender);

	int screencnt = 1, oneloopOffset = 5000;
	if (initParam.isSplitScreen) {
		screencnt = 2;
	}
	//��ʕ����������s
	for (int i = 0; i < screencnt; i++) {

		int offset = oneloopOffset * i;		

		if (initParam.isSplitScreen) {
			//��ʕ����Ȃ�J�����؂�ւ�
			m_renderManager.AddRender(0 + offset, m_cameraSwitchRender[i].get());
		}

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

		//SRGB�ɕϊ�
		m_renderManager.AddRender(8 + offset, &m_ConvertLinearToSRGB);

		//Effekseer�̕`��
		m_renderManager.AddRender(9 + offset, &m_effekseerRender);
		
		m_freeRenderPriority = 10;//�������火�܂Ŗ��g�p

		//�v���~�e�B�u�`��
		m_renderManager.AddRender(998 + offset, &m_primitiveRender);

#ifndef DW_MASTER
		//BUlletPhysics�̃f�o�b�O�`��
		m_renderManager.AddRender(999 + offset, &m_physicsDebugDrawRender);
#endif
		//�ŏI�`��
		m_renderManager.AddRender(1000 + offset, m_finalRender[i].get());		
	}

	//GPU�C�x���g�p
#ifndef DW_MASTER
	m_pd3dDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_pUserAnotation);
#endif
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