#include "DWstdafx.h"
#include "SoundEngine.h"

namespace DemolisherWeapon {

SoundEngine::SoundEngine()
{
}
SoundEngine::~SoundEngine()
{
	Release();
}

void SoundEngine::Init() {
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	HRESULT hr;

	if (FAILED(hr = XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
#ifndef DW_MASTER
		OutputDebugStringA("XAudio2Create‚ÉŽ¸”s‚µ‚Ü‚µ‚½B");
#endif
		return;
	}

	if (FAILED(hr = m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL))) {
#ifndef DW_MASTER
		OutputDebugStringA("CreateMasteringVoice‚ÉŽ¸”s‚µ‚Ü‚µ‚½B");
#endif
		return;
	}
}

void SoundEngine::Release() {
	m_pMasterVoice->DestroyVoice();
	m_pXAudio2->Release();
	CoUninitialize();
}

}