#include "DWstdafx.h"
#include "CSound.h"

namespace DemolisherWeapon {
namespace GameObj {

CSound::CSound(const wchar_t* fileName, bool isStreaming)
{
	m_isStreaming = isStreaming;

	if (!isStreaming) {
		m_wav = GetWAVManager().Load(fileName);
	}
	else {
		if (!InitStreaming(fileName)) { m_isStreaming = false; Error::Box("InitStreaming�Ɏ��s���܂���"); }
	}

	//������
	m_x3DEmitter.ChannelCount = m_isStreaming ? m_insWfx.nChannels : m_wav->wfx->nChannels;
	m_x3DEmitter.CurveDistanceScaler = 1000.0f;//������������͈�?
	m_emitterAzimuths.resize(m_x3DEmitter.ChannelCount);
	m_x3DEmitter.pChannelAzimuths = m_emitterAzimuths.data();
	for (auto& azi : m_emitterAzimuths) { azi = 0.0f; }
	//m_x3DEmitter.ChannelRadius = 1.0f;

	m_x3DDSPSettings.SrcChannelCount = m_x3DEmitter.ChannelCount;//���̃{�C�X�̃`�����l����
	m_x3DDSPSettings.DstChannelCount = GetEngine().GetSoundEngine().GetMasterVoiceDetails().InputChannels;//�]����̃{�C�X�̃`�����l����	
	m_matrixCoefficients.resize(m_x3DDSPSettings.SrcChannelCount * m_x3DDSPSettings.DstChannelCount);
	m_x3DDSPSettings.pMatrixCoefficients = m_matrixCoefficients.data();
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
}

void CSound::Play(bool enable3D) {

	m_is3D = enable3D;

	//�X�g���[�~���O�Đ�
	if (m_isStreaming) {
		StreamingPlay(); 
		return;
	}

	//�ʏ�Đ�
	if (m_sourceVoice || !m_wav)return;	

	HRESULT hr = GetEngine().GetSoundEngine().GetIXAudio2()->CreateSourceVoice(&m_sourceVoice, m_wav->wfx);
	if (FAILED(hr)) {
		char message[256];
		sprintf_s(message, "CreateSourceVoice()�Ɏ��s���܂���\nHRESULT:%x", hr);
		Error::Box(message);
		m_sourceVoice = nullptr;
		return;
	}

	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = m_wav->audioBytes;      //�o�b�t�@�̃o�C�g��
	buffer.pAudioData = m_wav->startAudio;      //�o�b�t�@�̐擪�A�h���X
	buffer.Flags = XAUDIO2_END_OF_STREAM;       // tell the source voice not to expect any data after this buffer
	m_sourceVoice->SubmitSourceBuffer(&buffer);

	m_sourceVoice->SetVolume(0.15f);

	m_sourceVoice->Start();
}

void CSound::Update() {
	if (!m_sourceVoice)return;

	while (m_isLockSourceVoice.exchange(true)) {}//�X�s�����b�N

	XAUDIO2_VOICE_STATE state;
	m_sourceVoice->GetState(&state);
	//�L���[�Ȃ��Ȃ�����Đ��~�߂�
	if (state.BuffersQueued <= 0) {
		m_isLockSourceVoice = false;//�X�s�����b�N����
		Release(); return;
	}

	if (m_is3D) {
		//3D�ݒ�X�V
		m_x3DEmitter.OrientFront = { 0,0,1 };
		m_x3DEmitter.OrientTop = { 0,1,0 };
		m_x3DEmitter.Position.x = m_3DPos.x;
		m_x3DEmitter.Position.y = m_3DPos.y;
		m_x3DEmitter.Position.z = m_3DPos.z;

		//3D�I�[�f�B�I�̌v�Z
		GetEngine().GetSoundEngine().X3DAudioCalculate(&m_x3DEmitter, &m_x3DDSPSettings);
		/*m_matrixCoefficients[0] = 1;
		m_matrixCoefficients[1] = 1;
		m_matrixCoefficients[2] = 0;
		m_matrixCoefficients[3] = 0;*/
		//�K�p
		m_sourceVoice->SetOutputMatrix(GetEngine().GetSoundEngine().GetMasterVoice(), m_x3DDSPSettings.SrcChannelCount, m_x3DDSPSettings.DstChannelCount, m_x3DDSPSettings.pMatrixCoefficients);
	}

	m_isLockSourceVoice = false;//�X�s�����b�N����
}

}
}