#pragma once

#include"system/GraphicsEngine.h"
#include"system/GameObject.h"

#include"FPS/CFpsCounter.h"

#include"Input/MouseCursor.h"
#include"Input/KeyState.h"
#include"Input/XInput.h"

#include"physics/Physics.h"

namespace DemolisherWeapon {

struct InitEngineParameter {
	float SDUnityChanScale = 1.0f;	//SD���j�e�B�����̑傫��(�����̊�l�ɂȂ�)
	int limitFps = 60;				//�t���[�����[�g���
	int standardFps = 60;			//����t���[�����[�g
	int	screenWidth = 1280;			//!<�X�N���[���̕��B
	int	screenHeight = 720;			//!<�X�N���[���̍����B
	int frameBufferWidth = 1280;	//!<�t���[���o�b�t�@�̕��B���ꂪ�����𑜓x�B
	int frameBufferHeight = 720;	//!<�t���[���o�b�t�@�̍����B���ꂪ�����𑜓x�B
	int refleshRate = 60;
	bool isWindowMode = TRUE;
};

class GameLoop {
public:
	GameLoop(GameObjectManager* gom, PhysicsWorld* m_physics) {
		m_gameObjectManager_Ptr = gom;
		m_physics_Ptr = m_physics;
	};

	void Init(int maxfps, int stdfps) {
		m_fpscounter.Init(maxfps, stdfps);
	}

	void Run();

	//�σt���[������(���̂�)
	void UnableVariableFramerateOnce() { m_noVariableFramerateOnce = true; }

	//FPS�����ݒ�
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_fpscounter.SetUseFpsLimiter(use, maxfps); }

	//����FPS���擾
	int GetStandardFrameRate()const { return m_fpscounter.GetStandardFrameRate(); }

private:
	bool DispatchWindowMessage();

	GameObjectManager* m_gameObjectManager_Ptr;
	PhysicsWorld* m_physics_Ptr;

	float m_runframecnt = 1.0f;
	bool m_noVariableFramerateOnce = true;
	CFpsCounter m_fpscounter;
};

class Engine
{

//�V���O���g��
private:
	Engine() : m_gameLoop(&m_gameObjectManager,&m_physics) {};// = default;
	~Engine() = default;
public:
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

public:

	//�C���X�^���X���擾
	static Engine& GetInstance()
	{
		static Engine* instance = nullptr;
		if (instance == nullptr) {
			instance = new Engine;
		}
		return *instance;
	}

	//�Q�[���̏������B
	void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, InitEngineParameter initParam);
	//�Q�[�����[�v
	void RunGameLoop() { m_gameLoop.Run(); };

	//�E�C���h�E�X�V
	void UpdateWindow() {
		GetWindowRect(GetWindowHandle(), &m_rect);
		m_winSize.x = (float)(m_rect.right - m_rect.left);
		m_winSize.y = (float)(m_rect.bottom - m_rect.top);
	}
	RECT GetWindowRECT()const { return m_rect; }
	CVector2 GetWindowSize()const { return m_winSize; }

	//�����X�P�[�����擾
	float GetDistanceScale()const { return m_distanceScale; }

	//�E�B���h�E�n���h���̎擾
	HWND& GetWindowHandle() {
		return m_hWnd;
	}
	//�O���t�B�b�N�X�G���W���̎擾
	GraphicsEngine& GetGraphicsEngine() {
		return m_graphicsEngine;
	}
	//�����G���W���̎擾
	PhysicsWorld& GetPhysicsWorld() {
		return m_physics;
	}
	//�Q�[���I�u�W�F�N�g�}�l�[�W���[�̎擾
	GameObjectManager& GetGameObjectManager() {
		return m_gameObjectManager;
	}	

	//�}�E�X�J�[�\���}�l�[�W���[�̎擾
	MouseCursor& GetMouseCursorManager() {
		return m_mouseCursorManager;
	}
	//�L�[�X�e�[�g�}�l�[�W���[�̎擾
	KeyState& GetKeyState() {
		return m_keyState;
	}
	//XInput�}�l�[�W���[�̎擾
	XInputManager& GetXInputManager() {
		return m_xinputManager;
	}	

	//�}�E�X�z�C�[����]��(�m�b�`��)�̃��Z�b�g
	void ResetMouseWheelNotch() {
		m_wheelNotch = 0;
		m_wheelHNotch = 0;
	}
	//�}�E�X�z�C�[����]��(�c)�̎擾
	int GetMouseWheelNotch() const { return m_wheelNotch; }
	//�}�E�X�z�C�[����]��(��)�̎擾
	int GetMouseWheel_H_Notch() const { return m_wheelHNotch; }

	//FPS�����ݒ�
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_gameLoop.SetUseFpsLimiter(use, maxfps); }
	//����FPS���擾
	int GetStandardFrameRate()const { return m_gameLoop.GetStandardFrameRate(); }

private:
	static LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);	

	//�E�C���h�E�̏�����
	void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, int winSizeW, int winSizeH);

	HWND m_hWnd;//�E�B���h�E�n���h��
	RECT m_rect;
	CVector2 m_winSize;

	//�����N���X
	GraphicsEngine m_graphicsEngine;
	PhysicsWorld m_physics;
	GameObjectManager m_gameObjectManager;
	GameLoop m_gameLoop;

	//���̓N���X
	MouseCursor m_mouseCursorManager;
	KeyState m_keyState;
	XInputManager m_xinputManager;
	static int m_wheelNotch, m_wheelRot;//�}�E�X�z�C�[���̉�]��(�c)
	static int m_wheelHNotch, m_wheelHRot;//�}�E�X�z�C�[���̉�]��(��)

	float m_distanceScale = 1.0f;//�����̃X�P�[��
};

//Engine�̃C���X�^���X���擾
static inline Engine& GetEngine()
{
	return Engine::GetInstance();
}

//3D���f�������_�[�ɕ`�悷�郂�f����o�^
static inline void AddDrawModelToD3Render(SkinModel* sm) 
{
	GetEngine().GetGraphicsEngine().AddDrawModelToD3Render(sm);
}
//�V���h�E�}�b�v�����_�[�ɕ`�悷�郂�f����o�^
static inline void AddDrawModelToShadowMapRender(SkinModel* sm)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToShadowMapRender(sm);
}

//���C���J�������擾
static inline GameObj::ICamera* GetMainCamera()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetMainCamera();
}
//���C���J������o�^
static inline void SetMainCamera(GameObj::ICamera* cam)
{
	GetEngine().GetGraphicsEngine().GetCameraManager().SetMainCamera(cam);
}

//�Q�[���I�u�W�F�N�g�̓o�^
template <typename T>
static inline void AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
}
/*template <typename T>
static inline T* AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
	return go;
}*/
//�Q�[���I�u�W�F�N�g�̍폜
/*static inline void DeleteGO(IGameObject* go) {
	if (go == nullptr) { return; }
	GetEngine().GetGameObjectManager().DeleteGameObj(go);
}*/

//�}�E�X�J�[�\���}�l�[�W���[���擾
static inline MouseCursor& GetMouseCursorManager() {
	return GetEngine().GetMouseCursorManager();
}
//�L�[�X�e�[�g�}�l�[�W���[���擾
static inline KeyState& GetKeyState() {
	return GetEngine().GetKeyState();
}
//XInput�}�l�[�W���[�̎擾
static inline XInputManager& GetXInputManager() {
	return GetEngine().GetXInputManager();
}

//�}�E�X�z�C�[����]��(�m�b�`��)�̃��Z�b�g
static inline void ResetMouseWheelNotch() {
	GetEngine().ResetMouseWheelNotch();
}
//�}�E�X�z�C�[����]��(�c)�̎擾
static inline int GetMouseWheelNotch() { return GetEngine().GetMouseWheelNotch(); }
//�}�E�X�z�C�[����]��(��)�̎擾
static inline int GetMouseWheel_H_Notch() { return GetEngine().GetMouseWheel_H_Notch(); }

//����t���[�����[�g���擾
static inline int GetStandardFrameRate() { return GetEngine().GetStandardFrameRate(); }

}