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

	// 3Dサウンド用リスナー設定の更新
	m_sound->SetListener({ 0.0f, 0.0f, 500.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	// 座標系の指定(RHで右手系、LHで左手系)
	m_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffekseerManager::Release() {
	// エフェクトの停止
	if (m_manager) m_manager->StopAllEffects();
	// エフェクトを解放します。再生中の場合は、再生が終了した後、自動的に解放されます。
	for (auto& effect : m_effects) {
		ES_SAFE_RELEASE(effect.second);
	}
	m_effects.clear();

	// エフェクト管理用インスタンスを破棄
	if (m_manager) m_manager->Destroy();
	// サウンド用インスタンスを破棄
	if (m_sound) m_sound->Destroy();
	// 描画用インスタンスを破棄
	if (m_renderer) m_renderer->Destroy();
}

void EffekseerManager::Update() {
	if (GetMainCamera()) {
		// 3Dサウンド用リスナー設定の更新
		m_sound->SetListener(GetMainCamera()->GetPos(), GetMainCamera()->GetTarget(), GetMainCamera()->GetUp());
	}

	// 再生中のエフェクトの移動等(::Effekseer::Manager経由で様々なパラメーターが設定できます。)
	//m_manager->AddLocation(handle, ::Effekseer::Vector3D);

	// 全てのエフェクトの更新
	m_manager->Update(60.0f / GetStandardFrameRate());
}

void EffekseerManager::Draw() {

#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "カメラが設定されていません!!", "Error", MB_OK);
		std::abort();
	}
#endif

	// 投影行列の更新
	m_renderer->SetProjectionMatrix(GetMainCamera()->GetProjMatrix());
	// カメラ行列の更新
	m_renderer->SetCameraMatrix(GetMainCamera()->GetViewMatrix());

	m_renderer->BeginRendering();
	m_manager->Draw();
	m_renderer->EndRendering();
}

Effekseer::Effect* EffekseerManager::Load(const wchar_t* filePath, float scale) {
	Effekseer::Effect* effect = nullptr;
	int index = Util::MakeHash(filePath);

	//既に登録されてないか?
	if (m_effects.count(index) > 0) {
		//登録されてたらマップから取得
		effect = m_effects[index];
	}
	else {
		//新規読み込み

		// エフェクトの読込	
		effect = Effekseer::Effect::Create(m_manager, (const EFK_CHAR*)filePath, scale);
		if (effect == nullptr) {
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "Effekseer::Effect::Create に失敗。\nFilePath:%ls", filePath);
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			return nullptr;
		}

		//エフェクトを登録
		m_effects[index] = effect;
	}

	return effect;
}

Effekseer::Handle EffekseerManager::Play(Effekseer::Effect* effect, const CVector3& pos) {
	// エフェクトの再生
	return m_manager->Play(effect,0.0f, 0.0f, 0.0f);// , pos.x, pos.y, pos.z); //Effekseer絶対許さねぇ…
}

void EffekseerManager::Stop(Effekseer::Handle handle) {
	// エフェクトの停止
	m_manager->StopEffect(handle);
}

}