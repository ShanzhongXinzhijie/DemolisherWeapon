#include "DWstdafx.h"
#include "EffekseerManager.h"

namespace DemolisherWeapon {

EffekseerManager::EffekseerManager()
{
}
EffekseerManager::~EffekseerManager()
{
	Release();
}

void EffekseerManager::Init() {

	// 描画管理インスタンスの生成
	m_renderer = ::EffekseerRendererDX11::Renderer::Create(GetEngine().GetGraphicsEngine().GetD3DDevice(), GetEngine().GetGraphicsEngine().GetD3DDeviceContext(), MAX_SPRITE);
	// サウンド管理インスタンスの生成
	m_sound = ::EffekseerSound::Sound::Create(GetEngine().GetSoundEngine().GetIXAudio2(), MAX_MONO_VOICE, MAX_STER_VOICE);
	// エフェクト管理用インスタンスの生成
	m_manager = ::Effekseer::Manager::Create(MAX_SPRITE);

	// 描画方法を指定します。独自に拡張することもできます。
	m_manager->SetSpriteRenderer(m_renderer->CreateSpriteRenderer());
	m_manager->SetRibbonRenderer(m_renderer->CreateRibbonRenderer());
	m_manager->SetRingRenderer(m_renderer->CreateRingRenderer());
	m_manager->SetTrackRenderer(m_renderer->CreateTrackRenderer());
	m_manager->SetModelRenderer(m_renderer->CreateModelRenderer());

	// テクスチャ画像の読込方法の指定(パッケージ等から読み込む場合拡張する必要があります。)
	m_manager->SetTextureLoader(m_renderer->CreateTextureLoader());

	// サウンド再生用インスタンスの指定
	m_manager->SetSoundPlayer(m_sound->CreateSoundPlayer());

	// サウンドデータの読込方法の指定(圧縮フォーマット、パッケージ等から読み込む場合拡張する必要があります。)
	m_manager->SetSoundLoader(m_sound->CreateSoundLoader());

	// 座標系の指定(RHで右手系、LHで左手系)
	m_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffekseerManager::Release() {
	// エフェクトを解放します。再生中の場合は、再生が終了した後、自動的に解放されます。
	ES_SAFE_RELEASE(effect);
	// エフェクト管理用インスタンスを破棄
	m_manager->Destroy();
	// サウンド用インスタンスを破棄
	m_sound->Destroy();
	// 描画用インスタンスを破棄
	m_renderer->Destroy();
}

void EffekseerManager::Update() {
	// 投影行列の更新
	m_renderer->SetProjectionMatrix(GetMainCamera()->GetProjMatrix());
	// カメラ行列の更新
	m_renderer->SetCameraMatrix(GetMainCamera()->GetViewMatrix());
	
	// 3Dサウンド用リスナー設定の更新
	m_sound->SetListener(GetMainCamera()->GetPos(), GetMainCamera()->GetTarget(), GetMainCamera()->GetUp());

	// 再生中のエフェクトの移動等(::Effekseer::Manager経由で様々なパラメーターが設定できます。)
	//m_manager->AddLocation(handle, ::Effekseer::Vector3D);

	// 全てのエフェクトの更新
	m_manager->Update();
}

void EffekseerRender::Render() {
	m_renderer->BeginRendering();
	m_manager->Draw();
	m_renderer->EndRendering();
}

// エフェクトの読込
Effekseer::Effect* effect = Effekseer::Effect::Create(manager, 読込先パス);
// エフェクトの再生
Effekseer::Handle handle = manager->Play(effect, 初期位置);

}