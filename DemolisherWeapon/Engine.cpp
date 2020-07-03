#include "DWstdafx.h"
#include "Engine.h"
#include "graphic/shader/ShaderPreloader.h"

namespace DemolisherWeapon {

Engine* Engine::instance = nullptr;

int Engine::m_wheelNotch = 0, Engine::m_wheelRot = 0;//マウスホイールの回転量(縦)
int Engine::m_wheelHNotch = 0, Engine::m_wheelHRot = 0;//マウスホイールの回転量(横)

///////////////////////////////////////////////////////////////////
//メッセージプロシージャ。
//hWndがメッセージを送ってきたウィンドウのハンドル。
//msgがメッセージの種類。
//wParamとlParamは引数。今は気にしなくてよい。
///////////////////////////////////////////////////////////////////
LRESULT CALLBACK Engine::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//送られてきたメッセージで処理を分岐させる。
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEWHEEL:
	{
		int w = GET_WHEEL_DELTA_WPARAM(wParam);
		if ((w > 0 && m_wheelRot < 0) || (w < 0 && m_wheelRot > 0)) {
			//方向変わってたら回転量リセット
			m_wheelRot = w;
		}
		else {
			//加算
			m_wheelRot += w;
		}
		//ノッチ数を求める
		m_wheelNotch = m_wheelRot / WHEEL_DELTA;
		//端数を保存する
		m_wheelRot %= WHEEL_DELTA;
	}
		break;
	case WM_MOUSEHWHEEL:
	{
		int w = GET_WHEEL_DELTA_WPARAM(wParam);
		if ((w > 0 && m_wheelHRot < 0) || (w < 0 && m_wheelHRot > 0)) {
			//方向変わってたら回転量リセット
			m_wheelHRot = w;
		}
		else {
			//加算
			m_wheelHRot += w;
		}
		//ノッチ数を求める
		m_wheelHNotch = m_wheelHRot / WHEEL_DELTA;
		//端数を保存する
		m_wheelHRot %= WHEEL_DELTA;
	}
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////
// ウィンドウの初期化。
///////////////////////////////////////////////////////////////////
void Engine::InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, const InitEngineParameter& initParam)
{
	//ウィンドウクラスのパラメータを設定(単なる構造体の変数の初期化です。)
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),		//構造体のサイズ。
		CS_CLASSDC,				//ウィンドウのスタイル。
								//ここの指定でスクロールバーをつけたりできるが、ゲームでは不要なのでCS_CLASSDCでよい。
		MsgProc,				//メッセージプロシージャ(後述)
		0,						//0でいい。
		0,						//0でいい。
		GetModuleHandle(NULL),	//このクラスのためのウインドウプロシージャがあるインスタンスハンドル。
								//何も気にしなくてよい。
		NULL,					//アイコンのハンドル。アイコンを変えたい場合ここを変更する。とりあえずこれでいい。
		NULL,					//マウスカーソルのハンドル。NULLの場合はデフォルト。
		NULL,					//ウィンドウの背景色。NULLの場合はデフォルト。
		NULL,					//メニュー名。NULLでいい。
		appName,				//ウィンドウクラスに付ける名前。
		NULL					//NULLでいい。
	};
	//ウィンドウクラスの登録。
	RegisterClassEx(&wc);

	//プライマリモニタの作業領域の取得
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	// ウィンドウの作成。
	m_hWnd = CreateWindow(
		appName,									//使用するウィンドウクラスの名前。
													//先ほど作成したウィンドウクラスと同じ名前にする。
		appName,									//ウィンドウの名前。ウィンドウクラスの名前と別名でもよい。
		initParam.windowStyle,						//ウィンドウスタイル。ゲームでは基本的にWS_OVERLAPPEDWINDOWでいい、
		(rect.left + rect.right) / 2 - initParam.screenWidth / 2,	//ウィンドウの初期X座標。
		(rect.top + rect.bottom) / 2 - initParam.screenHeight / 2,	//ウィンドウの初期Y座標。
		initParam.screenWidth,						//ウィンドウの幅。
		initParam.screenHeight,						//ウィンドウの高さ。
		NULL,										//親ウィンドウ。ゲームでは基本的にNULLでいい。
		NULL,										//メニュー。今はNULLでいい。
		hInstance,									//アプリケーションのインスタンス。
		NULL
	);

	//タイトルバーを含んだウィンドウサイズ算出
	RECT rx; //ウィンドウ領域
	RECT cx; //クライアント領域
	GetWindowRect(m_hWnd, &rx);
	GetClientRect(m_hWnd, &cx);
	const int new_width = initParam.screenWidth + (rx.right - rx.left) - (cx.right - cx.left);
	const int new_height = initParam.screenHeight + (rx.bottom - rx.top) - (cx.bottom - cx.top);
	
	//ウィンドウ位置・サイズ再設定
	SetWindowPos(m_hWnd, NULL, 
		(rect.left + rect.right) / 2 - new_width / 2,
		(rect.top + rect.bottom) / 2 - new_height / 2,
		new_width, new_height, NULL);

	//ウィンドウ表示
	ShowWindow(m_hWnd, nCmdShow);

	//ウィンドウ情報の更新
	UpdateWindow();
}


//ゲームの初期化。
void Engine::InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName, InitEngineParameter initParam)
{
	//ウィンドウを初期化。
	InitWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow, appName, initParam);

	//Bulletの初期化
	m_physics.Init();

	//DirectXの初期化。
#ifdef DW_DX12
	m_graphicsEngine.InitDx12(m_hWnd, initParam);
#else
	m_graphicsEngine.Init(m_hWnd, initParam);
#endif

	//XAudio2の初期化
	m_soundEngine.Init();

	//Effekseerの初期化
#ifndef DW_DX12
	m_effekseer.Init();
#endif

	//ゲームループの初期化
	m_gameLoop.Init(initParam.limitFps, initParam.standardFps, initParam.variableFpsMaxSec);

	//コリジョンマネージャーの初期化
	m_collisionManager = std::make_unique<GameObj::CollisionObjManager>();

	//距離のスケールを設定
	m_distanceScale = initParam.SDUnityChanScale;

	//アンビエントライト
	SetAmbientLight(initParam.defaultAmbientLight);

	//シェーダーのプリロード
	if (initParam.isPreloadPresetShader) {
		new GameObj::Suicider::ShaderPreloader;
	}
}

void Engine::ChangeWindowSize(int screenWidth, int screenHeight) {
	//タイトルバーを含んだウィンドウサイズ算出
	RECT rx; //ウィンドウ領域
	RECT cx; //クライアント領域
	GetWindowRect(m_hWnd, &rx);
	GetClientRect(m_hWnd, &cx);
	const int new_width = screenWidth + (rx.right - rx.left) - (cx.right - cx.left);
	const int new_height = screenHeight + (rx.bottom - rx.top) - (cx.bottom - cx.top);

	//プライマリモニタの作業領域の取得
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	//ウィンドウ位置・サイズ再設定
	SetWindowPos(m_hWnd, NULL,
		(rect.left + rect.right) / 2 - new_width / 2,
		(rect.top + rect.bottom) / 2 - new_height / 2,
		new_width, new_height, NULL);

	//ウィンドウ情報の更新
	UpdateWindow();
}

//Photonの初期化
void Engine::InitPhoton(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, PhotonNetworkLogic::EventActionFunc eventAction) {
	m_photon = std::make_unique<PhotonNetworkLogic>(appID, appVersion, eventAction);
}

//ウィンドウメッセージをディスパッチ。falseが返ってきたら、ゲーム終了。
bool GameLoop::DispatchWindowMessage()
{
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		//ウィンドウからのメッセージを受け取る。
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//ウィンドウメッセージが空になった。
			break;
		}
	}
	return msg.message != WM_QUIT;
}

//ゲームループ
void GameLoop::Run() {
	while (DispatchWindowMessage() == true) {

		//FPS計測
		m_fpscounter->Count();

		//ウィンドウ更新
		GetEngine().UpdateWindow();

		//マウスカーソル
		GetEngine().GetMouseCursorManager().Update();
		//キーステート
		GetEngine().GetKeyState().Update();
		//XInput
		GetEngine().GetXInputManager().Update();

		//実行時間を加算
		m_runframecnt += m_fpscounter->GetRunFrameCnt();

		//今回は可変フレームにしない
		//フラグがON or 1fの処理時間が上限設定より長い
		if (m_noVariableFramerateOnce || m_variableFpsMaxSec > -FLT_EPSILON && m_variableFpsMaxSec < m_fpscounter->GetFrameTimeSec()) {
			m_runframecnt = 1.0f;
			m_noVariableFramerateOnce = false;
		}

		//ゲームオブジェクトのループ前アプデ
		m_gameObjectManager_Ptr->PreLoopUpdate();

		//可変フレームループ
		m_ranGameLoopNum = 0;
		while((int)m_runframecnt >= 1){
			//通信
			if (GetPhoton()) { GetPhoton()->Update(); }

			//ゲームオブジェクト
			m_gameObjectManager_Ptr->Start();
			m_gameObjectManager_Ptr->Update();
			m_goNewDeleteManager_Ptr->FarewellDearDeadman();
			m_gameObjectManager_Ptr->Hell();

			//通信
			if (GetPhoton()) { GetPhoton()->Update(); }

			//各種なんか
			m_physics_Ptr->Update();
			GetEngine().GetSoundEngine().Update();
			m_effekseer_Ptr->Update();

			//操作系のなんか
			ResetMouseWheelNotch();//マウスホイールのリセット
			GetEngine().GetMouseCursorManager().ResetMouseMove();//マウス移動量のリセット			
			GetEngine().GetKeyState().InLoopUpdate();//キーステート
			GetEngine().GetXInputManager().InLoopUpdate();//XInput

			//実行回数カウンタ減らす
			m_runframecnt -= 1.0f;
			m_ranGameLoopNum++;
		}

		//ゲームオブジェクトのループ後アプデ
		m_gameObjectManager_Ptr->PostLoopUpdate();

		//ライト更新
		GetEngine().GetGraphicsEngine().GetLightManager().UpdateStructuredBuffers();
		GetEngine().GetGraphicsEngine().GetLightManager().UpdateConstantBuffer();

		//描画/////////////////////////////////////////////		

#ifndef DW_DX12
		//バックバッファをクリア
		GetEngine().GetGraphicsEngine().ClearBackBuffer();

		//3D用のビューポートにする
		GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().Get3DFrameBuffer_W(), GetEngine().GetGraphicsEngine().Get3DFrameBuffer_H());
#endif

		//レンダリング
		GetEngine().GetGraphicsEngine().RunRenderManager();

#ifndef DW_DX12
		//2D用の設定にする
		GetEngine().GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetEngine().GetGraphicsEngine().GetFrameBuffer_W(), GetEngine().GetGraphicsEngine().GetFrameBuffer_H());

		//2Dプリミティブの描画
		GetGraphicsEngine().GetPrimitiveRender().Render2D();
		GetGraphicsEngine().GetPrimitiveRender().PostRender2D();

		//ゲームオブジェクトによるポスト描画
		m_gameObjectManager_Ptr->PostRender();
		
		//FPS表示		
		m_fpscounter->Draw();

		//バックバッファを表へ
		GetEngine().GetGraphicsEngine().SwapBackBuffer();
#endif

		///////////////////////////////////////////////////

		//ゲーム終了
		if (m_loopBreak) {
			break;
		}

#ifndef DW_MASTER
		//シェーダーのホットリロード
		static float tttt = 0.0f;
		tttt += m_fpscounter->GetFrameTimeSec();
		if (tttt > 2.8f) {
			tttt = 0.0f;
#ifndef DW_DX12
			ShaderResources::GetInstance().HotReload();
#endif
		}
#endif

		//デバッグ操作
		if (m_isDebugInput) {
			//ゲーム終了
			if (GetAsyncKeyState(VK_ESCAPE)) {
				break;
			}
			//ポーズ
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