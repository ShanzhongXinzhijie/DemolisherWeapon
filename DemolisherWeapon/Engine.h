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
	float SDUnityChanScale = 1.0f;	//SDユニティちゃんの大きさ(距離の基準値になる)
									//メモ: 1m = 78.74fくらい?(Unityちゃんが全長1.5mくらいになる

	bool useFpsLimiter = true;		//フレームレートに上限を設定するか
	int limitFps = 60;				//フレームレート上限
	int standardFps = 60;			//動作フレームレート
	float variableFpsMaxSec = 1.0f;	//1描画フレームあたりの処理時間がこの秒数を超えると、可変フレームレート無効化(-FLT_EPSILON以下で無効化無効)
	
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;//ウィンドウスタイル
	int	screenWidth = 1280;			//ウィンドウの幅
	int	screenHeight = 720;			//ウィンドウの高さ
	int frameBufferWidth = 1280;	//フレームバッファの幅。これが内部解像度。
	int frameBufferHeight = 720;	//フレームバッファの高さ。これが内部解像度。
	int frameBufferWidth3D = 1280;	//3D描画の解像度(幅)
	int frameBufferHeight3D = 720;	//3D描画の解像度(高さ)
	int HUDWidth = 1280;			//HUDの解像度(幅)
	int HUDHeight = 720;			//HUDの解像度(高さ)
	//int refleshRate = 60;			//リフレッシュレート
	bool isWindowMode = true;		//ウィンドウモードか？
	bool useVSync = false;			//垂直同期を使用するか

	CVector3 defaultAmbientLight = {0.3f,0.3f,0.3f};//デフォルトのアンビエントライト

	EnSplitScreenMode isSplitScreen = enNoSplit;//画面分割するか

	//グラフィック設定
	EnShadowMapMode shadowMapSetting = enPCSS;	//シャドウマップ有効・PCSS
	bool isEnableSSAO = true;					//SSAO有効
	float SSAOBufferScale = 0.5f;				//SSAO解像度(3D描画の解像度に対する倍率)
	bool isEnableMotionBlur = true;				//モーションブラー有効
	bool isLensDistortion = true;				//歪曲収差
	bool isAntiAliasing = true;					//アンチエイリアス
	bool isEnableBloom = true;					//ブルーム有効
	float bloomBufferScale = 0.75f;				//ブルーム解像度(3D描画の解像度に対する倍率)
	bool isEnableDOF = false;					//被写界深度有効
	float DOFBufferScale = 0.5f;				//被写界深度解像度(3D描画の解像度に対する倍率)
	bool isAtmosphericFog = false;				//大気フォグ

	size_t xtk12DescriptorsMaxnum = 2048;		//DirectXTK12のリソース(フォントやスプライト)の最大数

	bool isPointFiltering = false;				//モデルの描画にポイントフィルタリングを使用するか
	bool isCreateMipmap = true;					//モデルテクスチャのミップマップを作成するか
	
	//シェーダのファイルパスをエンジンのものに置き換えるか?
	//※デバッグ用 DW_MASTER時無効
	bool isShaderPathReplaceForEngineFilePath = false;
	//更新があればシェーダーの再コンパイルを行うか?
	bool isShaderRecompile = true;
	//起動時にプリセットシェーダーのプリロードを行うか?
	bool isPreloadPresetShader = true;

	//低スペック用設定をセット
	void SetLowSpecSetting() {
		//解像度(フレームバッファ)
		frameBufferWidth3D = 640;
		frameBufferHeight3D = 360;
		//シャドウマップ無効
		shadowMapSetting = enON;
		//SSAO無効
		isEnableSSAO = false;
		//モーションブラー無効
		isEnableMotionBlur = false;
		//ブルーム無効
		isEnableBloom = false;
		//被写界深度無効
		isEnableDOF = false;
		//アンチエイリアス無効
		isAntiAliasing = false;
	}
	//中スペック用設定をセット
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

	//可変フレーム無効(一回のみ)
	void UnableVariableFramerateOnce() { m_noVariableFramerateOnce = true; }

	//FPS上限を設定
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_fpscounter->SetUseFpsLimiter(use, maxfps); }
	//動作基準FPSを設定
	void SetStandardFrameRate(int fps) { m_fpscounter->SetStandardFrameRate(fps); }

	//動作基準FPSを取得
	int GetStandardFrameRate()const { return m_fpscounter->GetStandardFrameRate(); }
	//1フレームの処理にかかった時間を取得
	float GetRealDeltaTimeSec()const { return m_fpscounter->GetFrameTimeSec(); }
	//1フレームに処理したゲームループ数を取得
	int RanGameLoopNum()const { return m_ranGameLoopNum; }

	//デバッグ情報を描画するか設定
	void SetIsDebugDraw(bool enable) { m_fpscounter->SetIsDebugDraw(enable); }
	bool GetIsDebugDraw()const { return m_fpscounter->GetIsDebugDraw(); }
	//debug操作を有効にするか設定
	void SetIsDebugInput(bool enable) { m_isDebugInput = enable; }
	bool GetIsDebugInput()const       { return m_isDebugInput; }

	//ゲームループを抜ける
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
//シングルトン
private:
	Engine() : m_gameLoop(&m_gameObjectManager,&m_goNewDeleteManager,&m_physics,&m_effekseer,m_fpscounter) {};
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
		if (instance == nullptr) {
			instance = new Engine;
		}
		return *instance;
	}
private:
	static Engine* instance;
	//インスタンスの削除
	static void DeleteInstance() {
		if (instance) {
			delete instance; instance = nullptr;
		}
	}
//

public:

	//ゲームの初期化。
	void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, InitEngineParameter initParam, HICON icon = NULL);

	//Photonの初期化
	void InitPhoton(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, PhotonNetworkLogic::EventActionFunc eventAction);
	//Photonの取得
	PhotonNetworkLogic* GetPhoton() {
		//DW_ERRORBOX(!m_photon, "GetPhoton() :Photonが初期化されていません");
		return m_photon.get();
	}

	//ゲームループ
	void RunGameLoop() { 
		m_gameLoop.Run(); 
		PostGameLoop();
	}

	void PostGameLoop() {
		//Photon削除
		//通信を切断するため
		m_photon.reset();
		//fpsカウンター削除
		//Sleepの精度をもとに戻すため
		m_gameLoop.Release();

		//DeleteInstance();
		return;
	}

	//ウィンドウサイズ変更
	void ChangeWindowSize(int screenWidth, int screenHeight);

	//ウインドウ更新
	void UpdateWindow() {
		GetWindowRect(GetWindowHandle(), &m_rect);
		m_winSize.x = (float)(m_rect.right - m_rect.left);
		m_winSize.y = (float)(m_rect.bottom - m_rect.top);

		GetClientRect(GetWindowHandle(), &m_clientRect);
		m_clientSize.x = (float)(m_clientRect.right - m_clientRect.left);
		m_clientSize.y = (float)(m_clientRect.bottom - m_clientRect.top);
	}
	RECT GetWindowRECT()const { return m_rect; }
	CVector2 GetWindowSize()const { return m_winSize; }

	RECT GetClientRECT()const { return m_clientRect; }
	CVector2 GetClientSize()const { return m_clientSize; }

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
	//サウンドエンジンの取得
	SoundEngine& GetSoundEngine() {
		return m_soundEngine;
	}
	//物理エンジンの取得
	CPhysicsWorld& GetPhysicsWorld() {
		return m_physics;
	}
	//Effekseerの取得
	EffekseerManager& GetEffekseer() {
		return m_effekseer;
	}
	//コリジョンマネージャーの取得
	GameObj::CollisionObjManager* GetCollisionObjManager() {
		return m_collisionManager.get();
	}
	//ゲームオブジェクトマネージャーの取得
	GameObjectManager& GetGameObjectManager() {
		return m_gameObjectManager;
	}	
	GONewDeleteManager& GetGONewDeleteManager() {
		return m_goNewDeleteManager;
	}

	//マウスカーソルマネージャーの取得
	CMouseCursor& GetMouseCursorManager() {
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
	//動作基準FPSを設定
	void SetStandardFrameRate(int fps) { m_gameLoop.SetStandardFrameRate(fps); }

	//動作基準FPSを取得
	int GetStandardFrameRate()const { return m_gameLoop.GetStandardFrameRate(); }
	//1フレームの処理にかかった時間を取得
	float GetRealDeltaTimeSec()const { return m_gameLoop.GetRealDeltaTimeSec(); }
	//1フレームに実行したゲームループ数を取得
	int RanGameLoopNum()const { return m_gameLoop.RanGameLoopNum(); }

	//可変フレーム無効(一回のみ)
	void UnableVariableFramerateOnce() { m_gameLoop.UnableVariableFramerateOnce(); }

	//デバッグ情報を描画するか設定
	void SetIsDebugDraw(bool enable) { m_gameLoop.SetIsDebugDraw(enable); }
	bool GetIsDebugDraw()const { return m_gameLoop.GetIsDebugDraw(); }
	//debug操作を有効にするか設定
	void SetIsDebugInput(bool enable) { m_gameLoop.SetIsDebugInput(enable); }
	bool GetIsDebugInput()const { return m_gameLoop.GetIsDebugInput(); }

	//ゲームループを抜ける
	void BreakGameLoop() { m_gameLoop.BreakGameLoop(); }

private:
	static LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);	

	//ウインドウの初期化
	void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, const InitEngineParameter& initParam, HICON icon);

	HWND m_hWnd;//ウィンドウハンドル
	RECT m_rect, m_clientRect;
	CVector2 m_winSize, m_clientSize;

	//処理クラス
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

	//入力クラス
	CMouseCursor m_mouseCursorManager;
	KeyState m_keyState;
	XInputManager m_xinputManager;
	static int m_wheelNotch, m_wheelRot;//マウスホイールの回転量(縦)
	static int m_wheelHNotch, m_wheelHRot;//マウスホイールの回転量(横)

	float m_distanceScale = 1.0f;//距離のスケール
};

//Engineのインスタンスを取得
inline Engine& GetEngine()
{
	return Engine::GetInstance();
}
//グラフィックスエンジンの取得
inline GraphicsEngine& GetGraphicsEngine()
{
	return GetEngine().GetGraphicsEngine();
}
//物理エンジンの取得
inline CPhysicsWorld& GetPhysicsWorld()
{
	return GetEngine().GetPhysicsWorld();
}
//Photonの取得
inline PhotonNetworkLogic* GetPhoton() {

	return GetEngine().GetPhoton();
}
//WAVManagerの取得
inline WAVManager& GetWAVManager() {

	return GetEngine().GetSoundEngine().GetWAVManager();
}
//WAVSettingManager(音声データ自体への設定を扱うクラス)の取得
inline WAVSettingManager& GetWAVSettingManager() {

	return GetEngine().GetSoundEngine().GetWAVSettingManager();
}

//使用しているグラフィックAPIを取得
inline EnGraphicsAPI GetGraphicsAPI()
{	
	return GetGraphicsEngine().GetUseAPI();
}

//3Dモデルレンダーに描画するモデルを登録
inline void AddDrawModelToD3Render(SkinModel* sm, int priority = DRAW_PRIORITY_DEFAULT, bool reverse = false)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToD3Render(sm, priority, reverse);
}
/// <summary>
/// シャドウマップレンダーに描画するモデルを登録
/// </summary>
/// <param name="sm">描画するモデル</param>
/// <param name="priority">描画順(0〜DRAW_PRIORITY_MAX)(大きいと後に描画)</param>
/// <param name="reverse">面方向を反転するか</param>
inline void AddDrawModelToShadowMapRender(SkinModel* sm, int priority = DRAW_PRIORITY_DEFAULT, bool reverse = true)
{
	GetEngine().GetGraphicsEngine().AddDrawModelToShadowMapRender(sm, priority, reverse);
}
//物理エンジンのデバッグ表示のモードを設定する
inline void SetPhysicsDebugDrawMode(int debugMode) {
	GetEngine().GetGraphicsEngine().SetPhysicsDebugDrawMode(debugMode);
}
//物理エンジンのデバッグ表示が有効か調べる
inline bool GetEnablePhysicsDebugDraw() {
	return GetEngine().GetGraphicsEngine().GetEnablePhysicsDebugDraw();
}

//メインカメラを取得
inline GameObj::ICamera* GetMainCamera()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetMainCamera();
}
//メインカメラを登録
inline void SetMainCamera(GameObj::ICamera* cam)
{
	GetEngine().GetGraphicsEngine().GetCameraManager().SetMainCamera(cam);
}
//カメラリストに登録
inline void SetCameraToList(int index, GameObj::ICamera* cam)
{
	GetEngine().GetGraphicsEngine().GetCameraManager().SetCameraToList(index, cam);
}
//カメラリストを取得
inline const std::vector<GameObj::ICamera*>& ViewCameraList()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().ViewCameraList();
}
//非推奨
[[deprecated("Please use SetCameraToList or ViewCameraList")]]
inline std::vector<GameObj::ICamera*>& GetCameraList()
{
	return GetEngine().GetGraphicsEngine().GetCameraManager().GetCameraList();
}
//画面分割数を取得
inline int GetScreenNum()
{
	return GetEngine().GetGraphicsEngine().GetSplitScreenMode() ? 2 : 1 ;
}

//LUTの設定
inline void SetLUT(int index, ID3D11ShaderResourceView* lut) {
	GetGraphicsEngine().GetFinalRender(index).SetLUT(lut);
}
//歪曲収差の使用を設定
inline void SetIsLensDistortion(bool enable)
{
	FinalRender::SetIsLensDistortion(enable);
}
//歪曲収差の度合いを設定
inline void SetLensDistortionScale(float scale)
{
	FinalRender::SetLensDistortionScale(scale);
}
//アンチエイリアスの使用を設定
inline void SetIsAntiAliasing(bool enable)
{
	FinalRender::SetIsAntiAliasing(enable);
}

//フォグの有効無効を設定
inline void SetEnableFog(bool enable) {
	GetGraphicsEngine().GetLightManager().SetEnableFog(enable);
}
//フォグが完全にかかる距離を設定
inline void SetFogDistance(float distance) {
	GetGraphicsEngine().GetLightManager().SetFogDistance(distance);
}
//フォグがかかり始める距離を設定
inline void SetFogStartDistance(float distance) {
	GetGraphicsEngine().GetLightManager().SetFogStartDistance(distance);
}
//高さフォグのかかり具合を設定
inline void SetFogHeightScale(float scale) {
	GetGraphicsEngine().GetLightManager().SetFogHeightScale(scale);
}
//フォグの色を設定
inline void SetFogColor(const CVector3& color) {
	GetGraphicsEngine().GetLightManager().SetFogColor(color);
}

//アンビエントライトを設定
inline void SetAmbientLight(const CVector3& color) {
	GetEngine().GetGraphicsEngine().GetDefferdRender().SetIsAmbientCubeMap(false);
	GetEngine().GetGraphicsEngine().GetLightManager().SetAmbientLight(color);
}
//アンビエントキューブマップを設定
//※MipMapが必要です(少なくとも8がいる)
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

//シャドウマップをひとつ有効化
//UINT width, UINT height ...シャドウマップのサイズ
inline CShadowMap* CreateShadowMap(UINT width, UINT height) {
	return GetEngine().GetGraphicsEngine().GetShadowMapRender().EnableShadowMap(width, height);
}

//ゲームオブジェクトの登録
template <typename T>
inline void AddGO(T* go) {
	GetEngine().GetGameObjectManager().AddGameObj(go);
}
//ゲームオブジェクトの作成
template<class T, class... TArgs>
inline T* NewGO(TArgs... ctorArgs)
{
	return GetEngine().GetGONewDeleteManager().NewGO<T>(ctorArgs...);
}
//ゲームオブジェクトの削除
//(ゲームオブジェクトの無効化フラグが立つ。実際にインスタンスが削除されるのは、全てのGOのPostUpdateが終わってから)
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
//即座にゲームオブジェクトを削除
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

//ゲームオブジェクトに名前をつける
inline void SetNameGO(IGameObject* go, const wchar_t* objectName)
{
	GetEngine().GetGameObjectManager().SetNameGO(go, objectName);
}
//ゲームオブジェクトの検索(単体)
template <typename T>
inline T* FindGO(const wchar_t* objectName) {
	return GetEngine().GetGameObjectManager().FindGO<T>(objectName);
}
template <typename T>
inline T* FindGO() {
	return GetEngine().GetGameObjectManager().FindGO<T>();
}
//ゲームオブジェクトの検索(複数)
//funcの戻り値=falseで検索終了
template<class T>
inline void QueryGOs(const wchar_t* objectName, std::function<bool(T* go)> func) {
	GetEngine().GetGameObjectManager().QueryGOs<T>(objectName, func);
}
template<class T>
inline void QueryGOs(std::function<bool(T* go)> func) {
	GetEngine().GetGameObjectManager().QueryGOs<T>(func);
}

/// <summary>
/// マネージャーに登録されているゲームオブジェクトの数を取得
/// </summary>
inline size_t GetRegisterGameObjectNum()
{
#ifndef DW_MASTER
	return GetEngine().GetGameObjectManager().GetGameObjNum();
#endif
	return 0;
}

//マウスカーソルマネージャーを取得
inline CMouseCursor& MouseCursor() {
	return GetEngine().GetMouseCursorManager();
}
inline CMouseCursor& GetMouseCursorManager() {
	return GetEngine().GetMouseCursorManager();
}
//キーステートマネージャーを取得
inline KeyState& GetKeyState() {
	return GetEngine().GetKeyState();
}
//キーボード入力を取得
inline bool GetKeyInput(int n) {
	return GetEngine().GetKeyState().GetInput(n);
}
inline bool GetKeyDown(int n) {
	return GetEngine().GetKeyState().GetDown(n);
}
inline bool GetKeyUp(int n) {
	return GetEngine().GetKeyState().GetUp(n);
}
//XInputマネージャーの取得
inline XInputManager& GetXInputManager() {
	return GetEngine().GetXInputManager();
}
//XInputパッドの取得
inline XInputPad& Pad(int padNo)
{
	return GetEngine().GetXInputManager().GetPad(padNo);
}

//マウスホイール回転数(ノッチ数)のリセット
inline void ResetMouseWheelNotch() {
	GetEngine().ResetMouseWheelNotch();
}
//マウスホイール回転量(縦)の取得
inline int GetMouseWheelNotch() { return GetEngine().GetMouseWheelNotch(); }
//マウスホイール回転量(横)の取得
inline int GetMouseWheel_H_Notch() { return GetEngine().GetMouseWheel_H_Notch(); }

//動作フレームレートを取得
inline int GetStandardFrameRate() { 
	return GetEngine().GetStandardFrameRate(); 
}
//1動作フレームの動作時間(秒)
inline float GetDeltaTimeSec() {
	return 1.0f / GetStandardFrameRate();
}
//1フレームの処理にかかった時間(秒)を取得
inline float GetRealDeltaTimeSec() {
	return GetEngine().GetRealDeltaTimeSec();
}

//コリジョンマネージャーに判定を追加
inline RegColObj* AddCollisionObj(GameObj::Suicider::CCollisionObj* obj) {
	return GetEngine().GetCollisionObjManager()->AddCollisionObj(obj);
}

//マスターボリュームを設定
inline void SetMasterVolume(float vol) {
	GetEngine().GetSoundEngine().GetMasterVoice()->SetVolume(vol);
}
inline float GetMasterVolume() {
	float vol;
	GetEngine().GetSoundEngine().GetMasterVoice()->GetVolume(&vol);
	return vol;
}

//線分の描画
inline void DrawLine3D(const CVector3& start, const CVector3& end, const CVector4& color, int HUDNum = -1) {
	GetGraphicsEngine().GetPrimitiveRender().AddLine(start, end, color, true, HUDNum);
}
inline void DrawLine2D(const CVector3& start, const CVector3& end, const CVector4& color, int HUDNum = -1) {
	GetGraphicsEngine().GetPrimitiveRender().AddLine(start, end, color, false, HUDNum);
}
//四角形の描画(2D)
inline void DrawQuad2D(const CVector3& min, const CVector3& max, const CVector4& color, int HUDNum = -1) {
	GetGraphicsEngine().GetPrimitiveRender().AddQuad(min, max, color, HUDNum);
}
//即時描画
//線分
inline void ImmediateDrawLine(const CVector3& start, const CVector3& end, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawLine(start, end, color, true);
}
inline void ImmediateDrawLine(const CVector2& start, const CVector2& end, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawLine(start, end, color, false);
}
//四角形
inline void ImmediateDrawQuad(const CVector3& min, const CVector3& max, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawQuad(min, max, color, true);
}
inline void ImmediateDrawQuad(const CVector2& min, const CVector2& max, const CVector4& color) {
	GetGraphicsEngine().GetPrimitiveRender().DrawQuad(min, max, color, false);
}

//デバッグ情報を描画するか設定
inline void SetIsDebugDraw(bool enable) { GetEngine().SetIsDebugDraw(enable); }
inline bool GetIsDebugDraw() { return GetEngine().GetIsDebugDraw(); }
//debug操作を有効にするか設定
inline void SetIsDebugInput(bool enable) { GetEngine().SetIsDebugInput(enable); }
inline bool GetIsDebugInput() { return GetEngine().GetIsDebugInput(); }

//ゲームループを抜ける
inline void BreakGameLoop() { GetEngine().BreakGameLoop(); }

}