#include "DWstdafx.h"
#include "SoundEngine.h"

namespace DemolisherWeapon {

SoundEngine::SoundEngine()
{
	memset(&m_x3DListener, 0, sizeof(X3DAUDIO_LISTENER));
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
		OutputDebugStringA("XAudio2Create�Ɏ��s���܂����B\n");
#endif
		return;
	}

	if (FAILED(hr = m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL))) {
#ifndef DW_MASTER
		OutputDebugStringA("CreateMasteringVoice�Ɏ��s���܂����B\n");
#endif
		return;
	}

	m_pMasterVoice->GetVoiceDetails(&m_masterVoiceDetails);

	DWORD dwChannelMask;
	if (FAILED(hr = m_pMasterVoice->GetChannelMask(&dwChannelMask))){
#ifndef DW_MASTER
		OutputDebugStringA("m_pMasterVoice->GetChannelMask�Ɏ��s���܂����B\n");
#endif
		return;
	}
	const float SPEEDOFSOUND = 340.29f;
	const float ONEMATER = 78.74f;
	if (FAILED(hr = X3DAudioInitialize(dwChannelMask, SPEEDOFSOUND * ONEMATER * GetEngine().GetDistanceScale(), m_x3DInstance))) {
#ifndef DW_MASTER
		OutputDebugStringA("X3DAudioInitialize�Ɏ��s���܂����B\n");
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