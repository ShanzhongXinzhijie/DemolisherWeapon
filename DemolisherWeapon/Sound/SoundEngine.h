#pragma once

#include "WAVManager.h"

namespace DemolisherWeapon {

class SoundEngine
{
public:
	SoundEngine();
	~SoundEngine();

	void Init();
	void Release();

	void Update();

	//3Dオーディオの計算をする
	void X3DAudioCalculate(const X3DAUDIO_EMITTER * emitter, X3DAUDIO_DSP_SETTINGS *DSPSettings);

	IXAudio2* GetIXAudio2(){ return m_pXAudio2; }

	IXAudio2MasteringVoice* GetMasterVoice() { return m_pMasterVoice; }
	const XAUDIO2_VOICE_DETAILS& GetMasterVoiceDetails()const { return m_masterVoiceDetails; }

	IXAudio2SubmixVoice* GetSubmixVoice() { return m_pSubmixVoice; }
	const XAUDIO2_VOICE_DETAILS& GetSubmixVoiceDetails()const { return m_submixVoiceDetails; }

	WAVManager& GetWAVManager() { return m_wavManager; }
	WAVSettingManager& GetWAVSettingManager() { return m_wavSettingManager; }

private:
	IXAudio2* m_pXAudio2 = nullptr;
	
	IXAudio2MasteringVoice* m_pMasterVoice = nullptr;
	XAUDIO2_VOICE_DETAILS m_masterVoiceDetails;

	IXAudio2SubmixVoice* m_pSubmixVoice = nullptr;
	XAUDIO2_VOICE_DETAILS m_submixVoiceDetails;

	X3DAUDIO_HANDLE m_x3DInstance;
	X3DAUDIO_LISTENER m_x3DListener;
	X3DAUDIO_CONE m_x3DListenerCone = { X3DAUDIO_PI*5.0f / 6.0f, X3DAUDIO_PI*11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };
	
	WAVManager m_wavManager;
	WAVSettingManager m_wavSettingManager;
};

}