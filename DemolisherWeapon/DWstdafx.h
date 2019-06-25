// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// または、参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する

//バージョン
#include"DWVersion.h"

// プログラムに必要な追加ヘッダーをここで参照してください
#include <WinSock2.h>
#include <windows.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <Xinput.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <xaudio2fx.h>

//Bullet
#ifndef DW_NON_BULLET
#include "bulletPhysics/src/btBulletDynamicsCommon.h"
#include "bulletPhysics/src/BulletCollision\CollisionDispatch\btGhostObject.h"
#endif

//DirectXTKのインクルードファイル。
#include "DirectXTK/Inc/Model.h"
#include "DirectXTK/Inc/Effects.h"	
#include "DirectXTK/Inc/CommonStates.h"
#include "DirectXTK/Inc/SpriteFont.h"
#include "DirectXTK/Inc/DDSTextureLoader.h"
#include "DirectXTK/Inc/WICTextureLoader.h"

//Engine
#include "Error/Error.h"

#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/Plane.h"

#include "Engine.h"
#include "system/GraphicsEngine.h"

#include "Graphic/Model/Render/CSkinModelRender.h"
#include "Graphic/Model/Render/CInstancingModelRender.h"
#include "Graphic/Font/CFont.h"
#include "Graphic/Sprite/CSprite.h"
#include "Graphic/Effekseer/CEffekseer.h"
#include "Graphic/Sky/CSkybox.h"
#include "Graphic/Billboard/CBillboard.h"
#include "Graphic/Billboard/CImposter.h"

#include "Graphic/Shadowmap/ShadowMapHandler.h"
#include "Graphic/Shadowmap/CascadeShadowHandler.h"

#include "level/Level.h"

#include "Sound/CSound.h"

namespace DemolisherWeapon {
	namespace SuicideObj = GameObj::Suicider;
}