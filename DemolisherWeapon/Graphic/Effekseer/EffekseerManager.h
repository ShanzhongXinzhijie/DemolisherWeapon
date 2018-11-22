#pragma once

#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include <EffekseerSoundXAudio2.h>

namespace DemolisherWeapon {

class EffekseerManager
{
public:
	EffekseerManager();
	~EffekseerManager();

	void Init();
	void Release();

	void Update();

private:
	static const int MAX_SPRITE = 2000;
	static const int MAX_MONO_VOICE = 20, MAX_STER_VOICE = 20;

	::EffekseerRenderer::Renderer* m_renderer = nullptr;
	::EffekseerSound::Sound* m_sound = nullptr;
	::Effekseer::Manager* m_manager = nullptr;
};

class EffekseerRender : public IRander {
public:
	void Render()override;
};

}