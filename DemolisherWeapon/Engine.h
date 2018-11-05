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
	float SDUnityChanScale = 1.0f;	//SDユニティちゃんの大きさ(距離の基準値になる)
	int limitFps = 60;				//フレームレート上限
	int standardFps = 60;			//動作フレームレート
	int	screenWidth = 1280;			//!<スクリーンの幅。
	int	screenHeight = 720;			//!<スクリーンの高さ。
	int frameBufferWidth = 1280;	//!<フレームバッファの幅。これが内部解像度。
	int frameBufferHeight = 720;	//!<フレームバッファの高さ。これが内部解像度。
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

	//可変フレーム無効(一回のみ)
	void UnableVariableFramerateOnce() { m_noVariableFramerateOnce = true; }

	//FPS上限を設定
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_fpscounter.SetUseFpsLimiter(use, maxfps); }

	//動作基準FPSを取得
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

//シングルトン
private:
	Engine() : m_gameLoop(&m_gameObjectManager,&m_physics) {};// = default;
	~Engine() = default;
public:
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

public:

	//インスタンスを取得
	static Engine& GetInstance()
	{
		static Engine* instance = nullptr;
		if (instance == nullptr) {
			instance = new Engine;
		}
		return *instance;
	}

	//ゲームの初期化。
	void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, InitEngineParameter initParam);
	//ゲームループ
	void RunGameLoop() { m_gameLoop.Run(); };

	//ウインドウ更新
	void UpdateWindow() {
		GetWindowRect(GetWindowHandle(), &m_rect);
		m_winSize.x = (float)(m_rect.right - m_rect.left);
		m_winSize.y = (float)(m_rect.bottom - m_rect.top);
	}
	RECT GetWindowRECT()const { return m_rect; }
	CVector2 GetWindowSize()const { return m_winSize; }

	//距離スケールを取得
	float GetDistanceScale()const { return m_distanceScale; }

	//ウィンドウハンドルの取得
	HWND& GetWindowHandle() {
		return m_hWnd;
	}
	//グラフィックスエンジンの取得
	GraphicsEngine& GetGraphicsEngine() {
		return m_graphicsEngine;
	}
	//物理エンジンの取得
	PhysicsWorld& GetPhysicsWorld() {
		return m_physics;
	}
	//ゲームオブジェクトマネージャーの取得
	GameObjectManager& GetGameObjectManager() {
		return m_gameObjectManager;
	}	

	//マウスカーソルマネージャーの取得
	MouseCursor& GetMouseCursorManager() {
		return m_mouseCursorManager;
	}
	//キーステートマネージャーの取得
	KeyState& GetKeyState() {
		return m_keyState;
	}
	//XInputマネージャーの取得
	XInputManager& GetXInputManager() {
		return m_xinputManager;
	}	

	//マウスホイール回転数(ノッチ数)のリセット
	void ResetMouseWheelNotch() {
		m_wheelNotch = 0;
		m_wheelHNotch = 0;
	}
	//マウスホイール回転量(縦)の取得
	int GetMouseWheelNotch() const { return m_wheelNotch; }
	//マウスホイール回転量(横)の取得
	int GetMouseWheel_H_Notch() const { return m_wheelHNotch; }

	//FPS上限を設定
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_gameLoop.SetUseFpsLimiter(use, maxfps); }
	//動作基準FPSを取得
	int GetStandardFrameRate()const { return m_gameLoop.GetStandardFrameRate(); }

private:
	static LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);	

	//ウインドウの初期化
	void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, int winSizeW, int winSizeH);

	HWND m_hWnd;//ウィンドウハンドル
	RECT m_rect;
	CVector2 m_winSize;

	//処理クラス
	GraphicsEngine m_graphicsEngine;
	PhysicsWorld m_physics;
	GameObjectManager m_gameObjectManager;
	GameLoop m_gameLoop;

	//入力クラス
	MouseCursor m_mouseCursorManager;
	KeyState m_keyState;
	XInputManager m_xinputManager;
	static int m_wheelNotch, m_wheelRot;//マウスホイールの回転量(縦)
	static int m_wheelHNotch, m_wheelHRot;//マウスホイールの回転量(横)

	float m_distanceScale = 1.0f;//距離のスケール
};

//Engineのインスタンスを取得
static inline Engine& GetEngine()
{
	return Engine::GetInstance();
}

//3Dモデルレンダーに描画するモデルを登録
static inline void AddDrawModelToD3Render(SkinModel* sm) 
{
	GetEngine().GetGraphicsEngine().AddDrawModelToD3Render(sm);
}
//シャドウマップレンダーに描画するモデルを登録
static inline void AddDrawModelToShadowMapRender(SkinModel* sm)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToShadowMapRender(sm);
}

//メインカメラを取得
static inline GameObj::ICamera* GetMainCamera()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetMainCamera();
}
//メインカメラを登録
static inline void SetMainCamera(GameObj::ICamera* cam)
{
	GetEngine().GetGraphicsEngine().GetCameraManager().SetMainCamera(cam);
}

//ゲームオブジェクトの登録
template <typename T>
static inline void AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
}
/*template <typename T>
static inline T* AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
	return go;
}*/
//ゲームオブジェクトの削除
/*static inline void DeleteGO(IGameObject* go) {
	if (go == nullptr) { return; }
	GetEngine().GetGameObjectManager().DeleteGameObj(go);
}*/

//マウスカーソルマネージャーを取得
static inline MouseCursor& GetMouseCursorManager() {
	return GetEngine().GetMouseCursorManager();
}
//キーステートマネージャーを取得
static inline KeyState& GetKeyState() {
	return GetEngine().GetKeyState();
}
//XInputマネージャーの取得
static inline XInputManager& GetXInputManager() {
	return GetEngine().GetXInputManager();
}

//マウスホイール回転数(ノッチ数)のリセット
static inline void ResetMouseWheelNotch() {
	GetEngine().ResetMouseWheelNotch();
}
//マウスホイール回転量(縦)の取得
static inline int GetMouseWheelNotch() { return GetEngine().GetMouseWheelNotch(); }
//マウスホイール回転量(横)の取得
static inline int GetMouseWheel_H_Notch() { return GetEngine().GetMouseWheel_H_Notch(); }

//動作フレームレートを取得
static inline int GetStandardFrameRate() { return GetEngine().GetStandardFrameRate(); }

}