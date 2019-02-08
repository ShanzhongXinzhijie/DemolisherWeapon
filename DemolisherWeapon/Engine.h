#pragma once

#include"system/GraphicsEngine.h"
#include"system/GameObject.h"

#include"Sound/SoundEngine.h"

#include"FPS/CFpsCounter.h"

#include"Input/MouseCursor.h"
#include"Input/KeyState.h"
#include"Input/XInput.h"

#include"physics/Physics.h"

#include "collision/CCollisionObj.h"

#include "Graphic/Effekseer/EffekseerManager.h"

#include"Network/PhotonManager.h"

#include"Render/ShadowMapRender.h"

namespace DemolisherWeapon {

static constexpr wchar_t DW_VER[] = L"���^���O���C����";//�G���W���̃o�[�W����

enum EnSplitScreenMode {
	enNoSplit=0,
	enVertical_TwoSplit,
	enSide_TwoSplit,
};

struct InitEngineParameter {
	float SDUnityChanScale = 1.0f;	//SD���j�e�B�����̑傫��(�����̊�l�ɂȂ�)
	//����: 1m = 78.74f���炢?(Unity����񂪑S��1.5m���炢�ɂȂ�
	int limitFps = 60;				//�t���[�����[�g���
	int standardFps = 60;			//����t���[�����[�g
	float variableFpsMaxSec = 1.0f;	//1�`��t���[��������̏������Ԃ����̕b���𒴂���ƁA�σt���[�����[�g������(-FLT_EPSILON�ȉ��Ŗ���������)
	int	screenWidth = 1280;			//�E�B���h�E�̕�
	int	screenHeight = 720;			//�E�B���h�E�̍���
	int frameBufferWidth = 1280;	//�t���[���o�b�t�@�̕��B���ꂪ�����𑜓x�B
	int frameBufferHeight = 720;	//�t���[���o�b�t�@�̍����B���ꂪ�����𑜓x�B
	int frameBufferWidth3D = 1280;	//3D�`��̉𑜓x(��)
	int frameBufferHeight3D = 720;	//3D�`��̉𑜓x(����)
	int refleshRate = 60;			//���t���b�V�����[�g
	bool isWindowMode = TRUE;		//�E�B���h�E���[�h���H

	CVector3 defaultAmbientLight = {0.3f,0.3f,0.3f};//�f�t�H���g�̃A���r�G���g���C�g

	EnSplitScreenMode isSplitScreen = enNoSplit;//��ʕ������邩

	//�O���t�B�b�N�ݒ�
	EnShadowMapMode shadowMapSetting = enPCSS;	//�V���h�E�}�b�v�L���EPCSS
	bool isEnableSSAO = true;					//SSAO�L��
	bool isEnableMotionBlur = true;				//���[�V�����u���[�L��

	//��X�y�b�N�p�ݒ���Z�b�g
	void SetLowSpecSetting() {
		//�𑜓x(�t���[���o�b�t�@)
		frameBufferWidth3D = 640;
		frameBufferHeight3D = 360;
		//�V���h�E�}�b�v����
		shadowMapSetting = enON;
		//SSAO����
		isEnableSSAO = false;
		//���[�V�����u���[����
		isEnableMotionBlur = false;
	}
	//���X�y�b�N�p�ݒ���Z�b�g
	void SetMiddleSpecSetting() {
		shadowMapSetting = enON;
	}
};

class GameLoop {
public:
	GameLoop(GameObjectManager* gom, GONewDeleteManager* gonewdel, CPhysicsWorld* physics, EffekseerManager* effekseer) {
		m_gameObjectManager_Ptr = gom;
		m_goNewDeleteManager_Ptr = gonewdel;
		m_physics_Ptr = physics;
		m_effekseer_Ptr = effekseer;
	};

	void Init(int maxfps, int stdfps, float variableFpsMaxSec) {
		m_fpscounter = std::make_unique<CFpsCounter>();
		m_fpscounter->Init(maxfps, stdfps);
		m_variableFpsMaxSec = variableFpsMaxSec;
	}

	void Release() {
		m_fpscounter.reset();
	}

	void Run();

	//�σt���[������(���̂�)
	void UnableVariableFramerateOnce() { m_noVariableFramerateOnce = true; }

	//FPS�����ݒ�
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_fpscounter->SetUseFpsLimiter(use, maxfps); }

	//����FPS���擾
	int GetStandardFrameRate()const { return m_fpscounter->GetStandardFrameRate(); }

	//�f�o�b�O����`�悷�邩�ݒ�
	void SetIsDebugDraw(bool enable) { m_fpscounter->SetIsDebugDraw(enable); }
	bool GetIsDebugDraw()const { return m_fpscounter->GetIsDebugDraw(); }
	//debug�����L���ɂ��邩�ݒ�
	void SetIsDebugInput(bool enable) { m_isDebugInput = enable; }
	bool GetIsDebugInput()const       { return m_isDebugInput; }

	//�Q�[�����[�v�𔲂���
	void BreakGameLoop() { m_loopBreak = true; }

private:
	bool DispatchWindowMessage();

	GameObjectManager* m_gameObjectManager_Ptr = nullptr;
	GONewDeleteManager* m_goNewDeleteManager_Ptr = nullptr;
	CPhysicsWorld* m_physics_Ptr = nullptr;
	EffekseerManager* m_effekseer_Ptr = nullptr;

	float m_runframecnt = 1.0f;
	bool m_noVariableFramerateOnce = true;
	float m_variableFpsMaxSec = -1.0f;
	std::unique_ptr<CFpsCounter> m_fpscounter;

	bool m_loopBreak = false;

#ifndef DW_MASTER
	bool m_isDebugInput = true;
#else
	bool m_isDebugInput = false;
#endif

};

class Engine
{

//�V���O���g��
private:
	Engine() : m_gameLoop(&m_gameObjectManager,&m_goNewDeleteManager,&m_physics,&m_effekseer) {};// = default;
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
		if (instance == nullptr) {
			instance = new Engine;
		}
		return *instance;
	}

private:

	static Engine* instance;

	//�C���X�^���X�̍폜
	static void DeleteInstance() {
		if (instance) {
			delete instance; instance = nullptr;
		}
	}

public:

	//�Q�[���̏������B
	void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, InitEngineParameter initParam);

	//Photon�̏�����
	void InitPhoton(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, PhotonNetworkLogic::EventActionFunc eventAction);
	//Photon�̎擾
	PhotonNetworkLogic* GetPhoton() {
		//DW_ERRORBOX(!m_photon, "GetPhoton() :Photon������������Ă��܂���");
		return m_photon.get();
	}

	//�Q�[�����[�v
	void RunGameLoop() { 
		m_gameLoop.Run(); 
		PostGameLoop();
	}

	void PostGameLoop() {
		//DeleteInstance();

		//Photon�폜
		//�ʐM��ؒf���邽��
		m_photon.reset();
		//fps�J�E���^�[�폜
		//Sleep�̐��x�����Ƃɖ߂�����
		m_gameLoop.Release();
	}

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
	//�T�E���h�G���W���̎擾
	SoundEngine& GetSoundEngine() {
		return m_soundEngine;
	}
	//�����G���W���̎擾
	CPhysicsWorld& GetPhysicsWorld() {
		return m_physics;
	}
	//Effekseer�̎擾
	EffekseerManager& GetEffekseer() {
		return m_effekseer;
	}
	//�R���W�����}�l�[�W���[�̎擾
	GameObj::CollisionObjManager* GetCollisionObjManager() {
		return m_collisionManager.get();
	}
	//�Q�[���I�u�W�F�N�g�}�l�[�W���[�̎擾
	GameObjectManager& GetGameObjectManager() {
		return m_gameObjectManager;
	}	
	GONewDeleteManager& GetGONewDeleteManager() {
		return m_goNewDeleteManager;
	}

	//�}�E�X�J�[�\���}�l�[�W���[�̎擾
	CMouseCursor& GetMouseCursorManager() {
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

	//�f�o�b�O����`�悷�邩�ݒ�
	void SetIsDebugDraw(bool enable) { m_gameLoop.SetIsDebugDraw(enable); }
	bool GetIsDebugDraw()const { return m_gameLoop.GetIsDebugDraw(); }
	//debug�����L���ɂ��邩�ݒ�
	void SetIsDebugInput(bool enable) { m_gameLoop.SetIsDebugInput(enable); }
	bool GetIsDebugInput()const { return m_gameLoop.GetIsDebugInput(); }

	//�Q�[�����[�v�𔲂���
	void BreakGameLoop() { m_gameLoop.BreakGameLoop(); }

private:
	static LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);	

	//�E�C���h�E�̏�����
	void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, int winSizeW, int winSizeH);

	HWND m_hWnd;//�E�B���h�E�n���h��
	RECT m_rect;
	CVector2 m_winSize;

	//�����N���X
	GraphicsEngine m_graphicsEngine;
	SoundEngine m_soundEngine;
	CPhysicsWorld m_physics;
	EffekseerManager m_effekseer;
	std::unique_ptr<GameObj::CollisionObjManager> m_collisionManager;
	std::unique_ptr<PhotonNetworkLogic> m_photon;
	GameObjectManager m_gameObjectManager;
	GONewDeleteManager m_goNewDeleteManager;
	GameLoop m_gameLoop;

	//���̓N���X
	CMouseCursor m_mouseCursorManager;
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
//�O���t�B�b�N�X�G���W���̎擾
static inline GraphicsEngine& GetGraphicsEngine()
{
	return GetEngine().GetGraphicsEngine();
}
//�����G���W���̎擾
static inline CPhysicsWorld& GetPhysicsWorld()
{
	return GetEngine().GetPhysicsWorld();
}
//Photon�̎擾
static inline PhotonNetworkLogic* GetPhoton() {

	return GetEngine().GetPhoton();
}
//WAVManager�̎擾
static inline WAVManager& GetWAVManager() {

	return GetEngine().GetSoundEngine().GetWAVManager();
}
//WAVSettingManager(�����f�[�^���̂ւ̐ݒ�������N���X)�̎擾
static inline WAVSettingManager& GetWAVSettingManager() {

	return GetEngine().GetSoundEngine().GetWAVSettingManager();
}

//3D���f�������_�[�ɕ`�悷�郂�f����o�^
static inline void AddDrawModelToD3Render(SkinModel* sm, int priority = DRAW_PRIORITY_DEFAULT)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToD3Render(sm, priority);
}
//�V���h�E�}�b�v�����_�[�ɕ`�悷�郂�f����o�^
static inline void AddDrawModelToShadowMapRender(SkinModel* sm, int priority = DRAW_PRIORITY_DEFAULT)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToShadowMapRender(sm, priority);
}
//�����G���W���̃f�o�b�O�\���̃��[�h��ݒ肷��
static inline void SetPhysicsDebugDrawMode(int debugMode) {
	GetEngine().GetGraphicsEngine().SetPhysicsDebugDrawMode(debugMode);
}
//�����G���W���̃f�o�b�O�\�����L�������ׂ�
static inline bool GetEnablePhysicsDebugDraw() {
	return GetEngine().GetGraphicsEngine().GetEnablePhysicsDebugDraw();
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
//�J�������X�g���擾
static inline std::vector<GameObj::ICamera*>& GetCameraList()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetCameraList();
}

//�A���r�G���g���C�g��ݒ�
static inline void SetAmbientLight(const CVector3& color) {
	GetEngine().GetGraphicsEngine().GetLightManager().SetAmbientLight(color);
}

//�V���h�E�}�b�v���ЂƂL����
//UINT width, UINT height ...�V���h�E�}�b�v�̃T�C�Y
static inline CShadowMap* CreateShadowMap(UINT width, UINT height) {
	return GetEngine().GetGraphicsEngine().GetShadowMapRender().EnableShadowMap(width, height);
}

//�Q�[���I�u�W�F�N�g�̓o�^
template <typename T>
static inline void AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
}
//�Q�[���I�u�W�F�N�g�̍쐬
template<class T, class... TArgs>
static inline T* NewGO(TArgs... ctorArgs)
{
	return GetEngine().GetGONewDeleteManager().NewGO<T>(ctorArgs...);
}
//�Q�[���I�u�W�F�N�g�̍폜
//(�Q�[���I�u�W�F�N�g�̖������t���O�����B���ۂɃC���X�^���X���폜�����̂́A�S�Ă�GO��PostUpdate���I����Ă���)
template <typename T>
static inline void DeleteGO(T*& go, bool newgoCheck = true)
{
	if (GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck)) {
		go = nullptr;
	}
}
static inline void DeleteGO(IGameObject* const go, bool newgoCheck = true)
{
	GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck);
}
//�����ɃQ�[���I�u�W�F�N�g���폜
template <typename T>
static inline void InstantDeleteGO(T*& go, bool newgoCheck = true)
{
	if (GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck, true)) {
		go = nullptr;
	}
}
static inline void InstantDeleteGO(IGameObject* const go, bool newgoCheck = true)
{
	GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck, true);
}

//�Q�[���I�u�W�F�N�g�ɖ��O������
static inline void SetNameGO(IGameObject* go, const wchar_t* objectName)
{
	GetEngine().GetGameObjectManager().SetNameGO(go, objectName);
}
//�Q�[���I�u�W�F�N�g�̌���(�P��)
template <typename T>
static inline T* FindGO(const wchar_t* objectName) {
	return GetEngine().GetGameObjectManager().FindGO<T>(objectName);
}
template <typename T>
static inline T* FindGO() {
	return GetEngine().GetGameObjectManager().FindGO<T>();
}
//�Q�[���I�u�W�F�N�g�̌���(����)
template<class T>
static inline void QueryGOs(const wchar_t* objectName, std::function<bool(T* go)> func) {
	GetEngine().GetGameObjectManager().QueryGOs<T>(objectName, func);
}
template<class T>
static inline void QueryGOs(std::function<bool(T* go)> func) {
	GetEngine().GetGameObjectManager().QueryGOs<T>(func);
}

//�}�E�X�J�[�\���}�l�[�W���[���擾
static inline CMouseCursor& MouseCursor() {
	return GetEngine().GetMouseCursorManager();
}
static inline CMouseCursor& GetMouseCursorManager() {
	return GetEngine().GetMouseCursorManager();
}
//�L�[�X�e�[�g�}�l�[�W���[���擾
static inline KeyState& GetKeyState() {
	return GetEngine().GetKeyState();
}
//�L�[�{�[�h���͂��擾
static inline bool GetKeyInput(int n) {
	return GetEngine().GetKeyState().GetInput(n);
}
static inline bool GetKeyDown(int n) {
	return GetEngine().GetKeyState().GetDown(n);
}
static inline bool GetKeyUp(int n) {
	return GetEngine().GetKeyState().GetUp(n);
}
//XInput�}�l�[�W���[�̎擾
static inline XInputManager& GetXInputManager() {
	return GetEngine().GetXInputManager();
}
//XInput�p�b�h�̎擾
static inline XInputPad& Pad(int padNo)
{
	return GetEngine().GetXInputManager().GetPad(padNo);
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

//1�t���[���̓��쎞��(�b)
static inline float GetDeltaTimeSec() {
	return 1.0f / GetStandardFrameRate();
}

//�R���W�����}�l�[�W���[�ɔ����ǉ�
static inline RegColObj* AddCollisionObj(GameObj::Suicider::CCollisionObj* obj) {
	return GetEngine().GetCollisionObjManager()->AddCollisionObj(obj);
}

//�}�X�^�[�{�����[����ݒ�
static inline void SetMasterVolume(float vol) {
	GetEngine().GetSoundEngine().GetMasterVoice()->SetVolume(vol);
}
static inline float GetMasterVolume() {
	float vol;
	GetEngine().GetSoundEngine().GetMasterVoice()->GetVolume(&vol);
	return vol;
}

//�f�o�b�O����`�悷�邩�ݒ�
static inline void SetIsDebugDraw(bool enable) { GetEngine().SetIsDebugDraw(enable); }
static inline bool GetIsDebugDraw() { return GetEngine().GetIsDebugDraw(); }
//debug�����L���ɂ��邩�ݒ�
static inline void SetIsDebugInput(bool enable) { GetEngine().SetIsDebugInput(enable); }
static inline bool GetIsDebugInput() { return GetEngine().GetIsDebugInput(); }

//�Q�[�����[�v�𔲂���
static inline void BreakGameLoop() { GetEngine().BreakGameLoop(); }

}