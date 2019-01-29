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

	WAVManager& GetWAVManager() { return m_wavManager; }

private:
	IXAudio2* m_pXAudio2 = nullptr;
	IXAudio2MasteringVoice* m_pMasterVoice = nullptr;
	XAUDIO2_VOICE_DETAILS m_masterVoiceDetails;

	X3DAUDIO_HANDLE m_x3DInstance;
	X3DAUDIO_LISTENER m_x3DListener;

	WAVManager m_wavManager;
};

}