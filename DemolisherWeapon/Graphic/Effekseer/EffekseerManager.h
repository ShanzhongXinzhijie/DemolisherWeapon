#pragma once

#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include <EffekseerSoundXAudio2.h>

#include <unordered_map>

namespace DemolisherWeapon {

class EffekseerManager
{
public:
	EffekseerManager();
	~EffekseerManager();

	void Init();
	void Release();

	void Update();
	void Draw();

	Effekseer::Effect* Load(const wchar_t* filePath, float scale);

	Effekseer::Handle Play(Effekseer::Effect* effect, const CVector3& pos);
	void Stop(Effekseer::Handle handle);

	Effekseer::Manager* GetManager() { return m_manager; }

private:
	static const int MAX_SPRITE = 12000;
	static const int MAX_MONO_VOICE = 64, MAX_STER_VOICE = 64;

	::EffekseerRenderer::Renderer* m_renderer = nullptr;
	::EffekseerSound::Sound* m_sound = nullptr;
	::Effekseer::Manager* m_manager = nullptr;

	std::unordered_map<int, Effekseer::Effect*> m_effects;
};

}