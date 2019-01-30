#include "DWstdafx.h"
#include "CSound.h"

namespace DemolisherWeapon {
namespace GameObj {

CSound::CSound(const wchar_t* fileName, bool isStreaming) {
	m_pSubmixVoice = GetEngine().GetSoundEngine().GetSubmixVoice();
	m_dstChannelcnt = GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputChannels;
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
			Error::Box("InitStreaming�Ɏ��s���܂���"); 
			return;
		}
	}

	m_pDataSetting = GetWAVSettingManager().Load(fileName);
	m_setting = *m_pDataSetting;

	//������
	m_x3DEmitter.ChannelCount = m_isStreaming ? m_insWfx.nChannels : m_wav->wfx->nChannels;
	m_x3DEmitter.CurveDistanceScaler = m_distance;//������������͈�?
	m_emitterAzimuths.resize(m_x3DEmitter.ChannelCount);
	m_x3DEmitter.pChannelAzimuths = m_emitterAzimuths.data();
	for (auto& azi : m_emitterAzimuths) { azi = 0.0f; }

	m_x3DDSPSettings.SrcChannelCount = m_x3DEmitter.ChannelCount;//���̃{�C�X�̃`�����l����
	m_x3DDSPSettings.DstChannelCount = m_dstChannelcnt;//�]����̃{�C�X�̃`�����l����	
	m_matrixCoefficients.resize(m_x3DDSPSettings.SrcChannelCount * m_x3DDSPSettings.DstChannelCount);
	m_x3DDSPSettings.pMatrixCoefficients = m_matrixCoefficients.data();

	m_defaultOutputMatrix.resize(m_x3DDSPSettings.SrcChannelCount * m_x3DDSPSettings.DstChannelCount);

	m_isInit = true;
}
CSound::~CSound()
{
	Release();

	if (m_thread.joinable()) {
		m_threadBreak = true;
		m_thread.join();
		m_threadBreak = false;
	}

	Release();
}

void CSound::Release() {

	ReleaseStreaming();

	if (m_thread.joinable()) { return; }

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

	//�|�[�Y����̍ĊJ
	if (m_isPause) {
		while (m_isLockSourceVoice.exchange(true)) {}//�X�s�����b�N
		m_sourceVoice->Start();
		m_isLockSourceVoice = false;//�X�s�����b�N����

		m_isPause = false;
		return;
	}

	if (m_sourceVoice) { return; }

	m_is3D = enable3D;

	//�X�g���[�~���O�Đ�
	if (m_isStreaming) {
		StreamingPlay(isLoop);
		return;
	}

	//�ʏ�Đ�
	if (m_sourceVoice || !m_wav)return;	

	XAUDIO2_SEND_DESCRIPTOR sendDescriptors[1];
	sendDescriptors[0].Flags = 0;
	sendDescriptors[0].pOutputVoice = m_pSubmixVoice;
	const XAUDIO2_VOICE_SENDS sendList = { 1, sendDescriptors };

	HRESULT hr = GetEngine().GetSoundEngine().GetIXAudio2()->CreateSourceVoice(&m_sourceVoice, m_wav->wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &sendList);
	if (FAILED(hr)) {
		char message[256];
		sprintf_s(message, "CreateSourceVoice()�Ɏ��s���܂���\nHRESULT:%x", hr);
		Error::Box(message);
		m_sourceVoice = nullptr;
		return;
	}

	//�f�t�H�s��ۑ�
	m_sourceVoice->GetOutputMatrix(m_pSubmixVoice, m_x3DDSPSettings.SrcChannelCount, m_x3DDSPSettings.DstChannelCount, m_defaultOutputMatrix.data());
	
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = m_wav->audioBytes;      //�o�b�t�@�̃o�C�g��
	buffer.pAudioData = m_wav->startAudio;      //�o�b�t�@�̐擪�A�h���X
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
	while (m_isLockSourceVoice.exchange(true)) {}//�X�s�����b�N

	if (!m_sourceVoice) { 
		m_isLockSourceVoice = false;//�X�s�����b�N����
		return;
	}

	m_sourceVoice->Stop(0);	

	m_isLockSourceVoice = false;//�X�s�����b�N����

	m_isPause = true;
}

void CSound::Update() {
	InUpdate();
}

void CSound::InUpdate(bool canStop) {

	//�X���b�h�I���҂�
	if (m_threadBreak && m_threadEnded){
		m_thread.join();
		m_threadBreak = false;
		m_threadEnded = false;

		Release();
	}
	//else if(m_willDelete){
	//	delete this;
	//}

	if (!m_sourceVoice)return;

	while (m_isLockSourceVoice.exchange(true)) {}//�X�s�����b�N

	if (canStop) {
		XAUDIO2_VOICE_STATE state;
		m_sourceVoice->GetState(&state);
		//�L���[�Ȃ��Ȃ�����Đ��~�߂�
		if (state.BuffersQueued <= 0) {
			m_isLockSourceVoice = false;//�X�s�����b�N����
			Release(); return;
		}
	}

	m_sourceVoice->SetVolume(m_setting.volume);
	m_sourceVoice->SetFrequencyRatio(m_setting.frequencyRatio);

	if (m_is3D) {
		//3D�ݒ�X�V
		m_x3DEmitter.OrientFront = { 0,0,1 };
		m_x3DEmitter.OrientTop = { 0,1,0 };
		m_x3DEmitter.Position.x = m_3DPos.x;
		m_x3DEmitter.Position.y = m_3DPos.y;
		m_x3DEmitter.Position.z = m_3DPos.z;

		//3D�I�[�f�B�I�̌v�Z
		GetEngine().GetSoundEngine().X3DAudioCalculate(&m_x3DEmitter, &m_x3DDSPSettings);		
	}
	else {
		//�f�t�H�s��擾
		std::memcpy(m_x3DDSPSettings.pMatrixCoefficients, m_defaultOutputMatrix.data(), m_defaultOutputMatrix.size() * sizeof(FLOAT32));

		//�`�����l�����Ƃ̃{�����[���ݒ�
		for (UINT32 x = 0; x < m_x3DDSPSettings.SrcChannelCount; x++) {
			for (UINT32 y = 0; y < m_x3DDSPSettings.DstChannelCount; y++) {
				m_x3DDSPSettings.pMatrixCoefficients[m_x3DDSPSettings.SrcChannelCount * y + x] *= m_setting.outChannelVolume[y];
			}
		}
	}

	//�K�p
	m_sourceVoice->SetOutputMatrix(m_pSubmixVoice, m_x3DDSPSettings.SrcChannelCount, m_x3DDSPSettings.DstChannelCount, m_x3DDSPSettings.pMatrixCoefficients);

	m_isLockSourceVoice = false;//�X�s�����b�N����
}

namespace Suicider {
	IXAudio2SubmixVoice*  CSE::m_SubmixVoice = nullptr;
	IXAudio2SubmixVoice* CBGM::m_SubmixVoice = nullptr;
}

}
}