#include "DWstdafx.h"
#include "Engine.h"
#include "graphic/shader/ShaderPreloader.h"

namespace DemolisherWeapon {

Engine* Engine::instance = nullptr;

int Engine::m_wheelNotch = 0, Engine::m_wheelRot = 0;//�}�E�X�z�C�[���̉�]��(�c)
int Engine::m_wheelHNotch = 0, Engine::m_wheelHRot = 0;//�}�E�X�z�C�[���̉�]��(��)

///////////////////////////////////////////////////////////////////
//���b�Z�[�W�v���V�[�W���B
//hWnd�����b�Z�[�W�𑗂��Ă����E�B���h�E�̃n���h���B
//msg�����b�Z�[�W�̎�ށB
//wParam��lParam�͈����B���͋C�ɂ��Ȃ��Ă悢�B
///////////////////////////////////////////////////////////////////
LRESULT CALLBACK Engine::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//�����Ă������b�Z�[�W�ŏ����𕪊򂳂���B
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEWHEEL:
	{
		int w = GET_WHEEL_DELTA_WPARAM(wParam);
		if ((w > 0 && m_wheelRot < 0) || (w < 0 && m_wheelRot > 0)) {
			//�����ς���Ă����]�ʃ��Z�b�g
			m_wheelRot = w;
		}
		else {
			//���Z
			m_wheelRot += w;
		}
		//�m�b�`�������߂�
		m_wheelNotch = m_wheelRot / WHEEL_DELTA;
		//�[����ۑ�����
		m_wheelRot %= WHEEL_DELTA;
	}
		break;
	case WM_MOUSEHWHEEL:
	{
		int w = GET_WHEEL_DELTA_WPARAM(wParam);
		if ((w > 0 && m_wheelHRot < 0) || (w < 0 && m_wheelHRot > 0)) {
			//�����ς���Ă����]�ʃ��Z�b�g
			m_wheelHRot = w;
		}
		else {
			//���Z
			m_wheelHRot += w;
		}
		//�m�b�`�������߂�
		m_wheelHNotch = m_wheelHRot / WHEEL_DELTA;
		//�[����ۑ�����
		m_wheelHRot %= WHEEL_DELTA;
	}
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////
// �E�B���h�E�̏������B
///////////////////////////////////////////////////////////////////
void Engine::InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, const InitEngineParameter& initParam)
{
	//�E�B���h�E�N���X�̃p�����[�^��ݒ�(�P�Ȃ�\���̂̕ϐ��̏������ł��B)
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),		//�\���̂̃T�C�Y�B
		CS_CLASSDC,				//�E�B���h�E�̃X�^�C���B
								//�����̎w��ŃX�N���[���o�[��������ł��邪�A�Q�[���ł͕s�v�Ȃ̂�CS_CLASSDC�ł悢�B
		MsgProc,				//���b�Z�[�W�v���V�[�W��(��q)
		0,						//0�ł����B
		0,						//0�ł����B
		GetModuleHandle(NULL),	//���̃N���X�̂��߂̃E�C���h�E�v���V�[�W��������C���X�^���X�n���h���B
								//�����C�ɂ��Ȃ��Ă悢�B
		NULL,					//�A�C�R���̃n���h���B�A�C�R����ς������ꍇ������ύX����B�Ƃ肠��������ł����B
		NULL,					//�}�E�X�J�[�\���̃n���h���BNULL�̏ꍇ�̓f�t�H���g�B
		NULL,					//�E�B���h�E�̔w�i�F�BNULL�̏ꍇ�̓f�t�H���g�B
		NULL,					//���j���[���BNULL�ł����B
		appName,				//�E�B���h�E�N���X�ɕt���閼�O�B
		NULL					//NULL�ł����B
	};
	//�E�B���h�E�N���X�̓o�^�B
	RegisterClassEx(&wc);

	//�v���C�}�����j�^�̍�Ɨ̈�̎擾
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	// �E�B���h�E�̍쐬�B
	m_hWnd = CreateWindow(
		appName,									//�g�p����E�B���h�E�N���X�̖��O�B
													//��قǍ쐬�����E�B���h�E�N���X�Ɠ������O�ɂ���B
		appName,									//�E�B���h�E�̖��O�B�E�B���h�E�N���X�̖��O�ƕʖ��ł��悢�B
		initParam.windowStyle,						//�E�B���h�E�X�^�C���B�Q�[���ł͊�{�I��WS_OVERLAPPEDWINDOW�ł����A
		(rect.left + rect.right) / 2 - initParam.screenWidth / 2,	//�E�B���h�E�̏���X���W�B
		(rect.top + rect.bottom) / 2 - initParam.screenHeight / 2,	//�E�B���h�E�̏���Y���W�B
		initParam.screenWidth,						//�E�B���h�E�̕��B
		initParam.screenHeight,						//�E�B���h�E�̍����B
		NULL,										//�e�E�B���h�E�B�Q�[���ł͊�{�I��NULL�ł����B
		NULL,										//���j���[�B����NULL�ł����B
		hInstance,									//�A�v���P�[�V�����̃C���X�^���X�B
		NULL
	);

	//�^�C�g���o�[���܂񂾃E�B���h�E�T�C�Y�Z�o
	RECT rx; //�E�B���h�E�̈�
	RECT cx; //�N���C�A���g�̈�
	GetWindowRect(m_hWnd, &rx);
	GetClientRect(m_hWnd, &cx);
	const int new_width = initParam.screenWidth + (rx.right - rx.left) - (cx.right - cx.left);
	const int new_height = initParam.screenHeight + (rx.bottom - rx.top) - (cx.bottom - cx.top);
	
	//�E�B���h�E�ʒu�E�T�C�Y�Đݒ�
	SetWindowPos(m_hWnd, NULL, 
		(rect.left + rect.right) / 2 - new_width / 2,
		(rect.top + rect.bottom) / 2 - new_height / 2,
		new_width, new_height, NULL);

	//�E�B���h�E�\��
	ShowWindow(m_hWnd, nCmdShow);

	//�E�B���h�E���̍X�V
	UpdateWindow();
}


//�Q�[���̏������B
void Engine::InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, InitEngineParameter initParam)
{
	//�E�B���h�E���������B
	InitWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow, appName, initParam);

	//Bullet�̏�����
	m_physics.Init();

	//DirectX�̏������B
#ifdef DW_DX12
	m_graphicsEngine.InitDx12(m_hWnd, initParam);
#else
	m_graphicsEngine.Init(m_hWnd, initParam);
#endif

	//XAudio2�̏�����
	m_soundEngine.Init();

	//Effekseer�̏�����
#ifndef DW_DX12
	m_effekseer.Init();
#endif

	//�Q�[�����[�v�̏�����
	m_gameLoop.Init(initParam.limitFps, initParam.standardFps, initParam.variableFpsMaxSec);

	//�R���W�����}�l�[�W���[�̏�����
	m_collisionManager = std::make_unique<GameObj::CollisionObjManager>();

	//�����̃X�P�[����ݒ�
	m_distanceScale = initParam.SDUnityChanScale;

	//�A���r�G���g���C�g
	SetAmbientLight(initParam.defaultAmbientLight);

	//�V�F�[�_�[�̃v�����[�h
	if (initParam.isPreloadPresetShader) {
		new GameObj::Suicider::ShaderPreloader;
	}
}

void Engine::ChangeWindowSize(int screenWidth, int screenHeight) {
	//�^�C�g���o�[���܂񂾃E�B���h�E�T�C�Y�Z�o
	RECT rx; //�E�B���h�E�̈�
	RECT cx; //�N���C�A���g�̈�
	GetWindowRect(m_hWnd, &rx);
	GetClientRect(m_hWnd, &cx);
	const int new_width = screenWidth + (rx.right - rx.left) - (cx.right - cx.left);
	const int new_height = screenHeight + (rx.bottom - rx.top) - (cx.bottom - cx.top);

	//�v���C�}�����j�^�̍�Ɨ̈�̎擾
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	//�E�B���h�E�ʒu�E�T�C�Y�Đݒ�
	SetWindowPos(m_hWnd, NULL,
		(rect.left + rect.right) / 2 - new_width / 2,
		(rect.top + rect.bottom) / 2 - new_height / 2,
		new_width, new_height, NULL);

	//�E�B���h�E���̍X�V
	UpdateWindow();
}

//Photon�̏�����
void Engine::InitPhoton(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, PhotonNetworkLogic::EventActionFunc eventAction) {
	m_photon = std::make_unique<PhotonNetworkLogic>(appID, appVersion, eventAction);
}

//�E�B���h�E���b�Z�[�W���f�B�X�p�b�`�Bfalse���Ԃ��Ă�����A�Q�[���I���B
bool GameLoop::DispatchWindowMessage()
{
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		//�E�B���h�E����̃��b�Z�[�W���󂯎��B
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//�E�B���h�E���b�Z�[�W����ɂȂ����B
			break;
		}
	}
	return msg.message != WM_QUIT;
}

//�Q�[�����[�v
void GameLoop::Run() {
	while (DispatchWindowMessage() == true) {

		//FPS�v��
		m_fpscounter->Count();

		//�E�B���h�E�X�V
		GetEngine().UpdateWindow();

		//�}�E�X�J�[�\��
		GetEngine().GetMouseCursorManager().Update();
		//�L�[�X�e�[�g
		GetEngine().GetKeyState().Update();
		//XInput
		GetEngine().GetXInputManager().Update();

		//���s���Ԃ����Z
		m_runframecnt += m_fpscounter->GetRunFrameCnt();

		//����͉σt���[���ɂ��Ȃ�
		//�t���O��ON or 1f�̏������Ԃ�����ݒ��蒷��
		if (m_noVariableFramerateOnce || m_variableFpsMaxSec > -FLT_EPSILON && m_variableFpsMaxSec < m_fpscounter->GetFrameTimeSec()) {
			m_runframecnt = 1.0f;
			m_noVariableFramerateOnce = false;
		}

		//�Q�[���I�u�W�F�N�g�̃��[�v�O�A�v�f
		m_gameObjectManager_Ptr->PreLoopUpdate();

		//�σt���[�����[�v
		m_ranGameLoopNum = 0;
		while((int)m_runframecnt >= 1){
			//�ʐM
			if (GetPhoton()) { GetPhoton()->Update(); }

			//�Q�[���I�u�W�F�N�g
			m_gameObjectManager_Ptr->Start();
			m_gameObjectManager_Ptr->Update();
			m_goNewDeleteManager_Ptr->FarewellDearDeadman();
			m_gameObjectManager_Ptr->Hell();

			//�ʐM
			if (GetPhoton()) { GetPhoton()->Update(); }

			//�e��Ȃ�
			m_physics_Ptr->Update();
			GetEngine().GetSoundEngine().Update();
			m_effekseer_Ptr->Update();

			//����n�̂Ȃ�
			ResetMouseWheelNotch();//�}�E�X�z�C�[���̃��Z�b�g
			GetEngine().GetMouseCursorManager().ResetMouseMove();//�}�E�X�ړ��ʂ̃��Z�b�g			
			GetEngine().GetKeyState().InLoopUpdate();//�L�[�X�e�[�g
			GetEngine().GetXInputManager().InLoopUpdate();//XInput

			//���s�񐔃J�E���^���炷
			m_runframecnt -= 1.0f;
			m_ranGameLoopNum++;
		}

		//�Q�[���I�u�W�F�N�g�̃��[�v��A�v�f
		m_gameObjectManager_Ptr->PostLoopUpdate();

		//���C�g�X�V
		GetEngine().GetGraphicsEngine().GetLightManager().UpdateStructuredBuffers();
		GetEngine().GetGraphicsEngine().GetLightManager().UpdateConstantBuffer();

		//�`��/////////////////////////////////////////////		

#ifndef DW_DX12
		//�o�b�N�o�b�t�@���N���A
		GetEngine().GetGraphicsEngine().ClearBackBuffer();

		//3D�p�̃r���[�|�[�g�ɂ���
		GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W(), GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H());
#endif

		//�����_�����O
		GetEngine().GetGraphicsEngine().RunRenderManager();

#ifndef DW_DX12
		//2D�p�̐ݒ�ɂ���
		GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

		//2D�v���~�e�B�u�̕`��
		GetGraphicsEngine().GetPrimitiveRender().Render2D();
		GetGraphicsEngine().GetPrimitiveRender().PostRender2D();

		//�Q�[���I�u�W�F�N�g�ɂ��|�X�g�`��
		m_gameObjectManager_Ptr->PostRender();
		
		//FPS�\��		
		m_fpscounter->Draw();

		//�o�b�N�o�b�t�@��\��
		GetEngine().GetGraphicsEngine().SwapBackBuffer();
#endif

		///////////////////////////////////////////////////

		//�Q�[���I��
		if (m_loopBreak) {
			break;
		}

#ifndef DW_MASTER
		//�V�F�[�_�[�̃z�b�g�����[�h
		static float tttt = 0.0f;
		tttt += m_fpscounter->GetFrameTimeSec();
		if (tttt > 2.8f) {
			tttt = 0.0f;
#ifndef DW_DX12
			ShaderResources::GetInstance().HotReload();
#endif
		}
#endif

		//�f�o�b�O����
		if (m_isDebugInput) {
			//�Q�[���I��
			if (GetAsyncKeyState(VK_ESCAPE)) {
				break;
			}
			//�|�[�Y
			if (GetAsyncKeyState(VK_F1)) {
				while (1) {
					Sleep(1);
					if (!GetAsyncKeyState(VK_F1)) { break; }
				}
			}
		}
	}
}

}