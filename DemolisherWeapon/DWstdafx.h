// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// または、参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する

#ifdef DW_DX12
#define DW_DX12_TEMPORARY
#else
#define DW_DX11
#endif

//バージョン
#include"DWVersion.h"

// プログラムに必要な追加ヘッダーをここで参照してください
#include <WinSock2.h>
#include <windows.h>

//TODO DX12版はそれのみインクルード
//ifdef DW_DX12
#include <D3d12.h>
#include "GraphicsAPI/DirectX12/d3dx12.h"
//else
#include <d3d11.h>
#include <d3d11_1.h>
//endif

#include <dxgi1_4.h>
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
#include <filesystem>

//Bullet
#ifndef DW_NON_BULLET
#include "bulletPhysics/src/btBulletDynamicsCommon.h"
#include "bulletPhysics/src/BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "bulletPhysics/src/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bulletPhysics/src/BulletSoftBody/btSoftBodyHelpers.h"
#endif

//DirectXTKのインクルードファイル。
#ifdef DW_DX12
#include "DirectXTK12/Inc/Model.h"
#include "DirectXTK12/Inc/Effects.h"	
#include "DirectXTK12/Inc/CommonStates.h"
#include "DirectXTK12/Inc/SpriteFont.h"
#include "DirectXTK12/Inc/DDSTextureLoader.h"
#include "DirectXTK12/Inc/WICTextureLoader.h"

#include "DirectXTK12/Src/PlatformHelpers.h"
#include "DirectXTK12/Inc/PrimitiveBatch.h"
#include "DirectXTK12/Audio/WAVFileReader.h"

#include "DirectXTK12/Inc/VertexTypes.h"
#include "DirectXTK12/Inc/ResourceUploadBatch.h"
#include "DirectXTK12/Inc/DescriptorHeap.h"
#include "DirectXTK12/Inc/DirectXHelpers.h"
#else
#include "DirectXTK/Inc/Model.h"
#include "DirectXTK/Inc/Effects.h"	
#include "DirectXTK/Inc/CommonStates.h"
#include "DirectXTK/Inc/SpriteFont.h"
#include "DirectXTK/Inc/DDSTextureLoader.h"
#include "DirectXTK/Inc/WICTextureLoader.h"

#include "DirectXTK/Src/PlatformHelpers.h"
#include "DirectXTK/Inc/PrimitiveBatch.h"
#include "DirectXTK/Audio/WAVFileReader.h"
#endif

//Engine
#include "Error/Error.h"
#include "Macro/Macro.h"

#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/Plane.h"

namespace DemolisherWeapon {
#include "Preset/Shader/ShadowMapHeader.h"
}

#include "Engine.h"
#include "system/GraphicsEngine.h"

#include "Graphic/Shader/StructuredBuffer.h"
#include "Graphic/Shader/ConstantBuffer.h"
#include "Graphic/Model/Render/CSkinModelRender.h"
#include "Graphic/Model/Render/CInstancingModelRender.h"
#include "Graphic/Model/Render/CRayTracingRender.h"
#include "Graphic/Font/CFont.h"
#include "Graphic/Sprite/CSprite.h"
#include "Graphic/Effekseer/CEffekseer.h"
#include "Graphic/Sky/CSkybox.h"
#include "Graphic/Billboard/CBillboard.h"
#include "Graphic/Billboard/CImposter.h"
#include "Graphic/Particle/CParticle.h"
#include "Graphic/CAABBRender.h"

#include "Graphic/Shadowmap/ShadowMapHandler.h"
#include "Graphic/Shadowmap/CascadeShadowHandler.h"

#include "level/Level.h"

#include "Sound/CSound.h"

#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {
	namespace SuicideObj = GameObj::Suicider;
}