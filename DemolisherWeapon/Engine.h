#pragma once

#include"system/GraphicsEngine.h"
#include"system/GameObject.h"

#include"Sound/SoundEngine.h"

#include"FPS/CFpsCounter.h"

#include"Input/MouseCursor.h"
#include"Input/KeyState.h"
#include"Input/XInput.h"

#include"physics/Physics.h"

#include"collision/CCollisionObj.h"

#include"Graphic/Effekseer/EffekseerManager.h"

#include"Network/PhotonManager.h"

#include"Render/ShadowMapRender.h"

namespace DemolisherWeapon {

struct InitEngineParameter {
	float SDUnityChanScale = 1.0f;	//SD���j�e�B�����̑傫��(�����̊�l�ɂȂ�)
									//����: 1m = 78.74f���炢?(Unity����񂪑S��1.5m���炢�ɂȂ�

	bool useFpsLimiter = true;		//�t���[�����[�g�ɏ����ݒ肷�邩
	int limitFps = 60;				//�t���[�����[�g���
	int standardFps = 60;			//����t���[�����[�g
	float variableFpsMaxSec = 1.0f;	//1�`��t���[��������̏������Ԃ����̕b���𒴂���ƁA�σt���[�����[�g������(-FLT_EPSILON�ȉ��Ŗ���������)
	
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;//�E�B���h�E�X�^�C��
	int	screenWidth = 1280;			//�E�B���h�E�̕�
	int	screenHeight = 720;			//�E�B���h�E�̍���
	int frameBufferWidth = 1280;	//�t���[���o�b�t�@�̕��B���ꂪ�����𑜓x�B
	int frameBufferHeight = 720;	//�t���[���o�b�t�@�̍����B���ꂪ�����𑜓x�B
	int frameBufferWidth3D = 1280;	//3D�`��̉𑜓x(��)
	int frameBufferHeight3D = 720;	//3D�`��̉𑜓x(����)
	int HUDWidth = 1280;			//HUD�̉𑜓x(��)
	int HUDHeight = 720;			//HUD�̉𑜓x(����)
	//int refleshRate = 60;			//���t���b�V�����[�g
	bool isWindowMode = true;		//�E�B���h�E���[�h���H
	bool useVSync = false;			//�����������g�p���邩

	CVector3 defaultAmbientLight = {0.3f,0.3f,0.3f};//�f�t�H���g�̃A���r�G���g���C�g

	EnSplitScreenMode isSplitScreen = enNoSplit;//��ʕ������邩

	//�O���t�B�b�N�ݒ�
	EnShadowMapMode shadowMapSetting = enPCSS;	//�V���h�E�}�b�v�L���EPCSS
	bool isEnableSSAO = true;					//SSAO�L��
	float SSAOBufferScale = 0.5f;				//SSAO�𑜓x(3D�`��̉𑜓x�ɑ΂���{��)
	bool isEnableMotionBlur = true;				//���[�V�����u���[�L��
	bool isLensDistortion = true;				//�c�Ȏ���
	bool isAntiAliasing = true;					//�A���`�G�C���A�X
	bool isEnableBloom = true;					//�u���[���L��
	float bloomBufferScale = 0.75f;				//�u���[���𑜓x(3D�`��̉𑜓x�ɑ΂���{��)
	bool isEnableDOF = false;					//��ʊE�[�x�L��
	float DOFBufferScale = 0.5f;				//��ʊE�[�x�𑜓x(3D�`��̉𑜓x�ɑ΂���{��)
	
	//�V�F�[�_�̃t�@�C���p�X���G���W���̂��̂ɒu�������邩?
	//���f�o�b�O�p DW_MASTER������
	bool isShaderPathReplaceForEngineFilePath = false;
	//�X�V������΃V�F�[�_�[�̍ăR���p�C�����s����?
	bool isShaderRecompile = true;
	//�N�����Ƀv���Z�b�g�V�F�[�_�[�̃v�����[�h���s����?
	bool isPreloadPresetShader = true;

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
		//�u���[������
		isEnableBloom = false;
		//��ʊE�[�x����
		isEnableDOF = false;
		//�A���`�G�C���A�X����
		isAntiAliasing = false;
	}
	//���X�y�b�N�p�ݒ���Z�b�g
	void SetMiddleSpecSetting() {
		shadowMapSetting = enON;
	}
};

class GameLoop {
public:
	GameLoop(GameObjectManager* gom, GONewDeleteManager* gonewdel, CPhysicsWorld* physics, EffekseerManager* effekseer, std::unique_ptr<CFpsCounter>& fpsCounter):
		m_gameObjectManager_Ptr(gom),
		m_goNewDeleteManager_Ptr(gonewdel),
		m_physics_Ptr(physics),
		m_effekseer_Ptr(effekseer),
		m_fpscounter(fpsCounter)
	{		
	};

	void Init(int maxfps, int stdfps, float variableFpsMaxSec, bool useFpsLimiter) {
		m_fpscounter = std::make_unique<CFpsCounter>();
		m_fpscounter->Init(maxfps, stdfps, useFpsLimiter);
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
	//����FPS��ݒ�
	void SetStandardFrameRate(int fps) { m_fpscounter->SetStandardFrameRate(fps); }

	//����FPS���擾
	int GetStandardFrameRate()const { return m_fpscounter->GetStandardFrameRate(); }
	//1�t���[���̏����ɂ����������Ԃ��擾
	float GetRealDeltaTimeSec()const { return m_fpscounter->GetFrameTimeSec(); }
	//1�t���[���ɏ��������Q�[�����[�v�����擾
	int RanGameLoopNum()const { return m_ranGameLoopNum; }

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

	int m_ranGameLoopNum = 0;
	float m_runframecnt = 1.0f;
	bool m_noVariableFramerateOnce = true;
	float m_variableFpsMaxSec = -1.0f;
	std::unique_ptr<CFpsCounter>& m_fpscounter;

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
	Engine() : m_gameLoop(&m_gameObjectManager,&m_goNewDeleteManager,&m_physics,&m_effekseer,m_fpscounter) {};
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
//

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
		//Photon�폜
		//�ʐM��ؒf���邽��
		m_photon.reset();
		//fps�J�E���^�[�폜
		//Sleep�̐��x�����Ƃɖ߂�����
		m_gameLoop.Release();

		//DeleteInstance();
		return;
	}

	//�E�B���h�E�T�C�Y�ύX
	void ChangeWindowSize(int screenWidth, int screenHeight);

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
	//����FPS��ݒ�
	void SetStandardFrameRate(int fps) { m_gameLoop.SetStandardFrameRate(fps); }

	//����FPS���擾
	int GetStandardFrameRate()const { return m_gameLoop.GetStandardFrameRate(); }
	//1�t���[���̏����ɂ����������Ԃ��擾
	float GetRealDeltaTimeSec()const { return m_gameLoop.GetRealDeltaTimeSec(); }
	//1�t���[���Ɏ��s�����Q�[�����[�v�����擾
	int RanGameLoopNum()const { return m_gameLoop.RanGameLoopNum(); }

	//�σt���[������(���̂�)
	void UnableVariableFramerateOnce() { m_gameLoop.UnableVariableFramerateOnce(); }

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
	void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, const InitEngineParameter& initParam);

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
	std::unique_ptr<CFpsCounter> m_fpscounter;

	//���̓N���X
	CMouseCursor m_mouseCursorManager;
	KeyState m_keyState;
	XInputManager m_xinputManager;
	static int m_wheelNotch, m_wheelRot;//�}�E�X�z�C�[���̉�]��(�c)
	static int m_wheelHNotch, m_wheelHRot;//�}�E�X�z�C�[���̉�]��(��)

	float m_distanceScale = 1.0f;//�����̃X�P�[��
};

//Engine�̃C���X�^���X���擾
inline Engine& GetEngine()
{
	return Engine::GetInstance();
}
//�O���t�B�b�N�X�G���W���̎擾
inline GraphicsEngine& GetGraphicsEngine()
{
	return GetEngine().GetGraphicsEngine();
}
//�����G���W���̎擾
inline CPhysicsWorld& GetPhysicsWorld()
{
	return GetEngine().GetPhysicsWorld();
}
//Photon�̎擾
inline PhotonNetworkLogic* GetPhoton() {

	return GetEngine().GetPhoton();
}
//WAVManager�̎擾
inline WAVManager& GetWAVManager() {

	return GetEngine().GetSoundEngine().GetWAVManager();
}
//WAVSettingManager(�����f�[�^���̂ւ̐ݒ�������N���X)�̎擾
inline WAVSettingManager& GetWAVSettingManager() {

	return GetEngine().GetSoundEngine().GetWAVSettingManager();
}

//3D���f�������_�[�ɕ`�悷�郂�f����o�^
inline void AddDrawModelToD3Render(SkinModel* sm, int priority = DRAW_PRIORITY_DEFAULT, bool reverse = false)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToD3Render(sm, priority, reverse);
}
/// <summary>
/// �V���h�E�}�b�v�����_�[�ɕ`�悷�郂�f����o�^
/// </summary>
/// <param name="sm">�`�悷�郂�f��</param>
/// <param name="priority">�`�揇(0�`DRAW_PRIORITY_MAX)(�傫���ƌ�ɕ`��)</param>
/// <param name="reverse">�ʕ����𔽓]���邩</param>
inline void AddDrawModelToShadowMapRender(SkinModel* sm, int priority = DRAW_PRIORITY_DEFAULT, bool reverse = true)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToShadowMapRender(sm, priority, reverse);
}
//�����G���W���̃f�o�b�O�\���̃��[�h��ݒ肷��
inline void SetPhysicsDebugDrawMode(int debugMode) {
	GetEngine().GetGraphicsEngine().SetPhysicsDebugDrawMode(debugMode);
}
//�����G���W���̃f�o�b�O�\�����L�������ׂ�
inline bool GetEnablePhysicsDebugDraw() {
	return GetEngine().GetGraphicsEngine().GetEnablePhysicsDebugDraw();
}

//���C���J�������擾
inline GameObj::ICamera* GetMainCamera()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetMainCamera();
}
//���C���J������o�^
inline void SetMainCamera(GameObj::ICamera* cam)
{
	GetEngine().GetGraphicsEngine().GetCameraManager().SetMainCamera(cam);
}
//�J�������X�g�ɓo�^
inline void SetCameraToList(int index, GameObj::ICamera* cam)
{
	GetEngine().GetGraphicsEngine().GetCameraManager().SetCameraToList(index, cam);
}
//�J�������X�g���擾
inline const std::vector<GameObj::ICamera*>& ViewCameraList()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().ViewCameraList();
}
//�񐄏�
[[deprecated("Please use SetCameraToList or ViewCameraList")]]
inline std::vector<GameObj::ICamera*>& GetCameraList()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetCameraList();
}
//��ʕ��������擾
inline int GetScreenNum()
{
	return GetEngine().GetGraphicsEngine().GetSplitScreenMode() ? 2 : 1 ;
}

//LUT�̐ݒ�
inline void SetLUT(int index, ID3D11ShaderResourceView* lut) {
	GetGraphicsEngine().GetFinalRender(index).SetLUT(lut);
}
//�c�Ȏ����̎g�p��ݒ�
inline void SetIsLensDistortion(bool enable)
{
	FinalRender::SetIsLensDistortion(enable);
}
//�c�Ȏ����̓x������ݒ�
inline void SetLensDistortionScale(float scale)
{
	FinalRender::SetLensDistortionScale(scale);
}
//�A���`�G�C���A�X�̎g�p��ݒ�
inline void SetIsAntiAliasing(bool enable)
{
	FinalRender::SetIsAntiAliasing(enable);
}

//�t�H�O�̗L��������ݒ�
inline void SetEnableFog(bool enable) {
	GetGraphicsEngine().GetLightManager().SetEnableFog(enable);
}
//�t�H�O�����S�ɂ����鋗����ݒ�
inline void SetFogDistance(float distance) {
	GetGraphicsEngine().GetLightManager().SetFogDistance(distance);
}
//�����t�H�O�̂�������ݒ�
inline void SetFogHeightScale(float scale) {
	GetGraphicsEngine().GetLightManager().SetFogHeightScale(scale);
}
//�t�H�O�̐F��ݒ�
inline void SetFogColor(const CVector3& color) {
	GetGraphicsEngine().GetLightManager().SetFogColor(color);
}

//�A���r�G���g���C�g��ݒ�
inline void SetAmbientLight(const CVector3& color) {
	GetEngine().GetGraphicsEngine().GetDefferdRender().SetIsAmbientCubeMap(false);
	GetEngine().GetGraphicsEngine().GetLightManager().SetAmbientLight(color);
}
//�A���r�G���g�L���[�u�}�b�v��ݒ�
//��MipMap���K�v�ł�(���Ȃ��Ƃ�8������)
inline void SetAmbientCubeMap(const wchar_t* filePass, const CVector3& scale) {
	GetEngine().GetGraphicsEngine().GetDefferdRender().SetIsAmbientCubeMap(true);
	GetEngine().GetGraphicsEngine().GetDefferdRender().SetAmbientCubeMap(filePass);
	GetEngine().GetGraphicsEngine().GetLightManager().SetAmbientLight(scale);
}
inline void SetAmbientCubeMap(ID3D11ShaderResourceView* srv, const CVector3& scale) {
	GetEngine().GetGraphicsEngine().GetDefferdRender().SetIsAmbientCubeMap(true);
	GetEngine().GetGraphicsEngine().GetDefferdRender().SetAmbientCubeMap(srv);
	GetEngine().GetGraphicsEngine().GetLightManager().SetAmbientLight(scale);
}

//�V���h�E�}�b�v���ЂƂL����
//UINT width, UINT height ...�V���h�E�}�b�v�̃T�C�Y
inline CShadowMap* CreateShadowMap(UINT width, UINT height) {
	return GetEngine().GetGraphicsEngine().GetShadowMapRender().EnableShadowMap(width, height);
}

//�Q�[���I�u�W�F�N�g�̓o�^
template <typename T>
inline void AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
}
//�Q�[���I�u�W�F�N�g�̍쐬
template<class T, class... TArgs>
inline T* NewGO(TArgs... ctorArgs)
{
	return GetEngine().GetGONewDeleteManager().NewGO<T>(ctorArgs...);
}
//�Q�[���I�u�W�F�N�g�̍폜
//(�Q�[���I�u�W�F�N�g�̖������t���O�����B���ۂɃC���X�^���X���폜�����̂́A�S�Ă�GO��PostUpdate���I����Ă���)
template <typename T>
inline void DeleteGO(T*& go, bool newgoCheck = true)
{
	if (GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck)) {
		go = nullptr;
	}
}
inline void DeleteGO(IGameObject* const go, bool newgoCheck = true)
{
	GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck);
}
//�����ɃQ�[���I�u�W�F�N�g���폜
template <typename T>
inline void InstantDeleteGO(T*& go, bool newgoCheck = true)
{
	if (GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck, true)) {
		go = nullptr;
	}
}
inline void InstantDeleteGO(IGameObject* const go, bool newgoCheck = true)
{
	GetEngine().GetGONewDeleteManager().DeleteGO(go, newgoCheck, true);
}

//�Q�[���I�u�W�F�N�g�ɖ��O������
inline void SetNameGO(IGameObject* go, const wchar_t* objectName)
{
	GetEngine().GetGameObjectManager().SetNameGO(go, objectName);
}
//�Q�[���I�u�W�F�N�g�̌���(�P��)
template <typename T>
inline T* FindGO(const wchar_t* objectName) {
	return GetEngine().GetGameObjectManager().FindGO<T>(objectName);
}
template <typename T>
inline T* FindGO() {
	return GetEngine().GetGameObjectManager().FindGO<T>();
}
//�Q�[���I�u�W�F�N�g�̌���(����)
//func�̖߂�l=false�Ō����I��
template<class T>
inline void QueryGOs(const wchar_t* objectName, std::function<bool(T* go)> func) {
	GetEngine().GetGameObjectManager().QueryGOs<T>(objectName, func);
}
template<class T>
inline void QueryGOs(std::function<bool(T* go)> func) {
	GetEngine().GetGameObjectManager().QueryGOs<T>(func);
}

/// <summary>
/// �}�l�[�W���[�ɓo�^����Ă���Q�[���I�u�W�F�N�g�̐����擾
/// </summary>
inline size_t GetRegisterGameObjectNum()
{
#ifndef DW_MASTER
	return GetEngine().GetGameObjectManager().GetGameObjNum();
#endif
	return 0;
}

//�}�E�X�J�[�\���}�l�[�W���[���擾
inline CMouseCursor& MouseCursor() {
	return GetEngine().GetMouseCursorManager();
}
inline CMouseCursor& GetMouseCursorManager() {
	return GetEngine().GetMouseCursorManager();
}
//�L�[�X�e�[�g�}�l�[�W���[���擾
inline KeyState& GetKeyState() {
	return GetEngine().GetKeyState();
}
//�L�[�{�[�h���͂��擾
inline bool GetKeyInput(int n) {
	return GetEngine().GetKeyState().GetInput(n);
}
inline bool GetKeyDown(int n) {
	return GetEngine().GetKeyState().GetDown(n);
}
inline bool GetKeyUp(int n) {
	return GetEngine().GetKeyState().GetUp(n);
}
//XInput�}�l�[�W���[�̎擾
inline XInputManager& GetXInputManager() {
	return GetEngine().GetXInputManager();
}
//XInput�p�b�h�̎擾
inline XInputPad& Pad(int padNo)
{
	return GetEngine().GetXInputManager().GetPad(padNo);
}

//�}�E�X�z�C�[����]��(�m�b�`��)�̃��Z�b�g
inline void ResetMouseWheelNotch() {
	GetEngine().ResetMouseWheelNotch();
}
//�}�E�X�z�C�[����]��(�c)�̎擾
inline int GetMouseWheelNotch() { return GetEngine().GetMouseWheelNotch(); }
//�}�E�X�z�C�[����]��(��)�̎擾
inline int GetMouseWheel_H_Notch() { return GetEngine().GetMouseWheel_H_Notch(); }

//����t���[�����[�g���擾
inline int GetStandardFrameRate() { 
	return GetEngine().GetStandardFrameRate(); 
}
//1����t���[���̓��쎞��(�b)
inline float GetDeltaTimeSec() {
	return 1.0f / GetStandardFrameRate();
}
//1�t���[���̏����ɂ�����������(�b)���擾
inline float GetRealDeltaTimeSec() {
	return GetEngine().GetRealDeltaTimeSec();
}

//�R���W�����}�l�[�W���[�ɔ����ǉ�
inline RegColObj* AddCollisionObj(GameObj::Suicider::CCollisionObj* obj) {
	return GetEngine().GetCollisionObjManager()->AddCollisionObj(obj);
}

//�}�X�^�[�{�����[����ݒ�
inline void SetMasterVolume(float vol) {
	GetEngine().GetSoundEngine().GetMasterVoice()->SetVolume(vol);
}
inline float GetMasterVolume() {
	float vol;
	GetEngine().GetSoundEngine().GetMasterVoice()->GetVolume(&vol);
	return vol;
}

//�����̕`��
inline void DrawLine3D(const CVector3& start, const CVector3& end, const CVector4& color, int HUDNum = -1) {
	GetGraphicsEngine().GetPrimitiveRender().AddLine(start, end, color, true, HUDNum);
}
inline void DrawLine2D(const CVector3& start, const CVector3& end, const CVector4& color, int HUDNum = -1) {
	GetGraphicsEngine().GetPrimitiveRender().AddLine(start, end, color, false, HUDNum);
}
//�l�p�`�̕`��(2D)
inline void DrawQuad2D(const CVector3& min, const CVector3& max, const CVector4& color, int HUDNum = -1) {
	GetGraphicsEngine().GetPrimitiveRender().AddQuad(min, max, color, HUDNum);
}
//�����`��
//����
inline void ImmediateDrawLine(const CVector3& start, const CVector3& end, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawLine(start, end, color, true);
}
inline void ImmediateDrawLine(const CVector2& start, const CVector2& end, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawLine(start, end, color, false);
}
//�l�p�`
inline void ImmediateDrawQuad(const CVector3& min, const CVector3& max, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawQuad(min, max, color, true);
}
inline void ImmediateDrawQuad(const CVector2& min, const CVector2& max, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawQuad(min, max, color, false);
}

//�f�o�b�O����`�悷�邩�ݒ�
inline void SetIsDebugDraw(bool enable) { GetEngine().SetIsDebugDraw(enable); }
inline bool GetIsDebugDraw() { return GetEngine().GetIsDebugDraw(); }
//debug�����L���ɂ��邩�ݒ�
inline void SetIsDebugInput(bool enable) { GetEngine().SetIsDebugInput(enable); }
inline bool GetIsDebugInput() { return GetEngine().GetIsDebugInput(); }

//�Q�[�����[�v�𔲂���
inline void BreakGameLoop() { GetEngine().BreakGameLoop(); }

}