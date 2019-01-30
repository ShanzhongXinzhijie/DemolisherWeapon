#include "DWstdafx.h"
#include "SoundEngine.h"

namespace DemolisherWeapon {

SoundEngine::SoundEngine()
{
	memset(&m_x3DListener, 0, sizeof(X3DAUDIO_LISTENER));
	m_x3DListener.pCone = &m_x3DListenerCone;
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
		OutputDebugStringA("XAudio2Create‚ÉŽ¸”s‚µ‚Ü‚µ‚½B\n");
#endif
		return;
	}

	if (FAILED(hr = m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL))) {
#ifndef DW_MASTER
		OutputDebugStringA("CreateMasteringVoice‚ÉŽ¸”s‚µ‚Ü‚µ‚½B\n");
#endif
		return;
	}
	m_pMasterVoice->GetVoiceDetails(&m_masterVoiceDetails);

	if (FAILED(hr = m_pXAudio2->CreateSubmixVoice(&m_pSubmixVoice, MAX_CHANNEL, GetEngine().GetSoundEngine().GetMasterVoiceDetails().InputSampleRate))) {
#ifndef DW_MASTER
		OutputDebugStringA("CreateSubmixVoice‚ÉŽ¸”s‚µ‚Ü‚µ‚½B\n");
#endif
		return;
	}
	m_pSubmixVoice->GetVoiceDetails(&m_submixVoiceDetails);

	DWORD dwChannelMask;
	if (FAILED(hr = m_pMasterVoice->GetChannelMask(&dwChannelMask))){
#ifndef DW_MASTER
		OutputDebugStringA("m_pMasterVoice->GetChannelMask‚ÉŽ¸”s‚µ‚Ü‚µ‚½B\n");
#endif
		return;
	}
	const float SPEEDOFSOUND = 340.29f;
	const float ONEMATER = 78.74f;
	if (FAILED(hr = X3DAudioInitialize(dwChannelMask, SPEEDOFSOUND * ONEMATER * GetEngine().GetDistanceScale(), m_x3DInstance))) {
#ifndef DW_MASTER
		OutputDebugStringA("X3DAudioInitialize‚ÉŽ¸”s‚µ‚Ü‚µ‚½B\n");
#endif
		return;
	}
}

void SoundEngine::Release() {
	m_pMasterVoice->DestroyVoice();
	m_pXAudio2->Release();
	CoUninitialize();
}

void SoundEngine::Update() {
	if (GetMainCamera()) {
		CVector3 front = GetMainCamera()->GetTarget() - GetMainCamera()->GetPos(); front.Normalize();
		m_x3DListener.OrientFront.x = front.x;
		m_x3DListener.OrientFront.y = front.y;
		m_x3DListener.OrientFront.z = front.z;

		m_x3DListener.OrientTop.x = GetMainCamera()->GetUp().x;
		m_x3DListener.OrientTop.y = GetMainCamera()->GetUp().y;
		m_x3DListener.OrientTop.z = GetMainCamera()->GetUp().z;

		m_x3DListener.Position.x = GetMainCamera()->GetPos().x;
		m_x3DListener.Position.y = GetMainCamera()->GetPos().y;
		m_x3DListener.Position.z = GetMainCamera()->GetPos().z;
	}
}

void SoundEngine::X3DAudioCalculate(const X3DAUDIO_EMITTER * emitter, X3DAUDIO_DSP_SETTINGS *DSPSettings) {
	::X3DAudioCalculate(m_x3DInstance, &m_x3DListener, emitter, X3DAUDIO_CALCULATE_MATRIX, DSPSettings);
}

}