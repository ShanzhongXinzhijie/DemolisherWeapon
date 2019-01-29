#include "DWstdafx.h"
#include "CSound.h"

namespace DemolisherWeapon {
namespace GameObj {

CSound::CSound(const wchar_t* fileName, bool isStreaming) {
	Init(fileName, isStreaming);
}

void CSound::Init(const wchar_t* fileName, bool isStreaming)
{
	if (m_sourceVoice) { return; }

	m_isInit = false;

	m_isStreaming = isStreaming;

	if (!isStreaming) {
		m_wav = GetWAVManager().Load(fileName);
		if (!m_wav) {
			return;
		}
	}
	else {
		if (!InitStreaming(fileName)) { 
			m_isStreaming = false; 
			Error::Box("InitStreamingに失敗しました"); 
			return;
		}
	}

	//初期化
	m_x3DEmitter.ChannelCount = m_isStreaming ? m_insWfx.nChannels : m_wav->wfx->nChannels;
	m_x3DEmitter.CurveDistanceScaler = m_distance;//音が聞こえる範囲?
	m_emitterAzimuths.resize(m_x3DEmitter.ChannelCount);
	m_x3DEmitter.pChannelAzimuths = m_emitterAzimuths.data();
	for (auto& azi : m_emitterAzimuths) { azi = 0.0f; }

	m_x3DDSPSettings.SrcChannelCount = m_x3DEmitter.ChannelCount;//このボイスのチャンネル数
	m_x3DDSPSettings.DstChannelCount = GetEngine().GetSoundEngine().GetMasterVoiceDetails().InputChannels;//転送先のボイスのチャンネル数	
	m_matrixCoefficients.resize(m_x3DDSPSettings.SrcChannelCount * m_x3DDSPSettings.DstChannelCount);
	m_x3DDSPSettings.pMatrixCoefficients = m_matrixCoefficients.data();

	m_isInit = true;
}
CSound::~CSound()
{
	Release();
}

void CSound::Release() {

	ReleaseStreaming();

	//m_wav = nullptr;
	if (m_sourceVoice) { 
		m_sourceVoice->Stop(0);
		//m_sourceVoice->FlushSourceBuffers();
		m_sourceVoice->DestroyVoice(); 
		m_sourceVoice = nullptr;
	}	

	m_is3D = false;
	m_isPause = false;
}

void CSound::Play(bool enable3D, bool isLoop) {

	if (!m_isInit) { return; }

	//ポーズからの再開
	if (m_isPause) {
		while (m_isLockSourceVoice.exchange(true)) {}//スピンロック
		m_sourceVoice->Start();
		m_isLockSourceVoice = false;//スピンロック解除

		m_isPause = false;
		return;
	}

	m_is3D = enable3D;

	//ストリーミング再生
	if (m_isStreaming) {
		StreamingPlay(isLoop);
		return;
	}

	//通常再生
	if (m_sourceVoice || !m_wav)return;	

	HRESULT hr = GetEngine().GetSoundEngine().GetIXAudio2()->CreateSourceVoice(&m_sourceVoice, m_wav->wfx);
	if (FAILED(hr)) {
		char message[256];
		sprintf_s(message, "CreateSourceVoice()に失敗しました\nHRESULT:%x", hr);
		Error::Box(message);
		m_sourceVoice = nullptr;
		return;
	}

	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = m_wav->audioBytes;      //バッファのバイト数
	buffer.pAudioData = m_wav->startAudio;      //バッファの先頭アドレス
	buffer.Flags = XAUDIO2_END_OF_STREAM;       // tell the source voice not to expect any data after this buffer
	if (isLoop) {
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	m_sourceVoice->SubmitSourceBuffer(&buffer);

	InUpdate();

	m_sourceVoice->Start();
}

void CSound::Stop() {
	Release();
}
void CSound::Pause() {
	while (m_isLockSourceVoice.exchange(true)) {}//スピンロック

	if (!m_sourceVoice) { 
		m_isLockSourceVoice = false;//スピンロック解除
		return;
	}

	m_sourceVoice->Stop(0);	

	m_isLockSourceVoice = false;//スピンロック解除

	m_isPause = true;
}

void CSound::Update() {
	InUpdate();
}

void CSound::InUpdate(bool canStop) {
	if (!m_sourceVoice)return;

	while (m_isLockSourceVoice.exchange(true)) {}//スピンロック

	if (canStop) {
		XAUDIO2_VOICE_STATE state;
		m_sourceVoice->GetState(&state);
		//キューなくなったら再生止める
		if (state.BuffersQueued <= 0) {
			m_isLockSourceVoice = false;//スピンロック解除
			Release(); return;
		}
	}

	m_sourceVoice->SetVolume(m_volume);

	if (m_is3D) {
		//3D設定更新
		m_x3DEmitter.OrientFront = { 0,0,1 };
		m_x3DEmitter.OrientTop = { 0,1,0 };
		m_x3DEmitter.Position.x = m_3DPos.x;
		m_x3DEmitter.Position.y = m_3DPos.y;
		m_x3DEmitter.Position.z = m_3DPos.z;

		//3Dオーディオの計算
		GetEngine().GetSoundEngine().X3DAudioCalculate(&m_x3DEmitter, &m_x3DDSPSettings);		

		//適用
		m_sourceVoice->SetOutputMatrix(GetEngine().GetSoundEngine().GetMasterVoice(), m_x3DDSPSettings.SrcChannelCount, m_x3DDSPSettings.DstChannelCount, m_x3DDSPSettings.pMatrixCoefficients);
	}

	m_isLockSourceVoice = false;//スピンロック解除
}

}
}