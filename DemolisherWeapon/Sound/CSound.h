#pragma once

#include <thread>

namespace DemolisherWeapon {

	struct StreamingVoiceContext : public IXAudio2VoiceCallback
	{
		STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override
		{
		}
		STDMETHOD_(void, OnVoiceProcessingPassEnd)() override
		{
		}
		STDMETHOD_(void, OnStreamEnd)() override
		{
		}
		STDMETHOD_(void, OnBufferStart)(void*) override
		{
		}
		STDMETHOD_(void, OnBufferEnd)(void*) override
		{
			SetEvent(hBufferEndEvent);
		}
		STDMETHOD_(void, OnLoopEnd)(void*) override
		{
		}
		STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override
		{
		}

		HANDLE hBufferEndEvent;

		StreamingVoiceContext() :
			hBufferEndEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE))
		{
		}
		virtual ~StreamingVoiceContext()
		{
			CloseHandle(hBufferEndEvent);
		}
	};	

namespace GameObj {

	class CSound : public IGameObject
	{
	public:
		CSound() {}
		CSound(const wchar_t* fileName, bool isStreaming = false);
		virtual ~CSound();

		void Init(const wchar_t* fileName, bool isStreaming = false);

		//�X�g���[�~���O�Đ��̏I����҂��Ď��E����
		/*void Delete() {
			Release();
			m_willDelete = true;
		}*/

		//������
		void Update()override;

		//�Đ�
		void Play(bool enable3D = false, bool isLoop = false);		
		//�����~�߂�
		//���X�g���[�~���O�Đ��͂����ɂ͒�~���܂���
		void Stop();
		//�ꎞ��~
		void Pause();

		//3D�Đ��p�p�����[�^�[��ݒ�
		void SetPos(const CVector3& pos) { m_3DPos = pos; }
		void SetDistance(float dis) { m_distance = dis; }

		//�ݒ���擾
		WAVSettingManager::WAVSetting* GetSetting() {
			return &m_setting;
		}
		void SetVolume(float vol) { m_setting.volume = vol; }
		void SetFrequencyRatio(float ratio) { m_setting.frequencyRatio = ratio; }
		void SetOutChannelVolume(int channel, float vol) { m_setting.outChannelVolume[channel] = vol; }

		//���f�[�^���̂ւ̐ݒ���擾
		WAVSettingManager::WAVSetting* GetDataSetting() {
			return m_pDataSetting;
		}
		void SetDataVolume(float vol) { m_pDataSetting->volume = vol; SetVolume(vol); }
		void SetDataFrequencyRatio(float ratio) { m_pDataSetting->frequencyRatio = ratio; SetDataFrequencyRatio(ratio); }
		void SetDataOutChannelVolume(int channel, float vol) { m_pDataSetting->outChannelVolume[channel] = vol; SetOutChannelVolume(channel, vol); }

		void SetUseSubmixVoice(IXAudio2SubmixVoice* submix, UINT32 channelCnt) {
			m_pSubmixVoice = submix;
			m_dstChannelcnt = channelCnt;
		}

		//�Đ������擾(�ꎞ��~�͍l������Ȃ�)
		bool GetIsPlaying()const { return m_sourceVoice; }
		//�Đ����ňꎞ��~�����ĂȂ����擾
		bool GetIsPlayingAndNotPause()const { return !m_isPause && m_sourceVoice; }
		//�ꎞ��~�����擾
		bool GetIsPausing()const { return m_isPause; }

		bool GetIsStreaming()const { return m_isStreaming; }

	private:
		void Release();
		void InUpdate(bool canStop = true);

		bool InitStreaming(const wchar_t* fileName);
		void ReleaseStreaming();
		void StreamingPlay(bool isLoop);
		void Streaming();
		
	private:		
		bool m_isInit = false;
		//bool m_willDelete = false;

		WAVManager::WAVData* m_wav = nullptr;
		IXAudio2SourceVoice* m_sourceVoice = nullptr;
		IXAudio2SubmixVoice* m_pSubmixVoice = nullptr;
		UINT32 m_dstChannelcnt = 0;

		bool m_isPause = false;

		WAVSettingManager::WAVSetting* m_pDataSetting = nullptr;
		WAVSettingManager::WAVSetting m_setting;

		std::vector<FLOAT32> m_defaultOutputMatrix;

		//3D�Đ��p
		bool m_is3D = false;
		X3DAUDIO_EMITTER m_x3DEmitter = { 0 };
		X3DAUDIO_DSP_SETTINGS m_x3DDSPSettings = { 0 };
		std::vector<FLOAT32> m_matrixCoefficients;
		std::vector<FLOAT32> m_emitterAzimuths;
		CVector3 m_3DPos;
		float m_distance = 1000.0f;

		//�X�g���[�~���O�p
		bool m_isStreaming = false;
		bool m_isStreamingLoop = false;
		std::thread m_thread;
		std::atomic<bool> m_threadBreak = false;
		std::atomic<bool> m_isLockSourceVoice = false;
		bool m_threadEnded = false;
		std::wstring m_fileName;
		WAVEFORMATEX m_insWfx;
		DWORD m_insStartAudio;
		uint32_t m_insAudioBytes;
		HANDLE m_async = INVALID_HANDLE_VALUE;
		StreamingVoiceContext m_voiceContext;
		OVERLAPPED m_ovlCurrentRequest = { 0 };

		static constexpr int STREAMING_BUFFER_SIZE = 65536;
		static constexpr int MAX_BUFFER_COUNT = 3;
		static_assert((STREAMING_BUFFER_SIZE % 2048) == 0, "Streaming size must be 2K aligned to use for async I/O");

		BYTE m_buffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE] = {0};
	};
	
namespace Suicider {
	class CSE : public CSound
	{
	public:
		CSE(const wchar_t* fileName, bool isStreaming = false) : CSound::CSound(fileName, isStreaming) {
			SetUseSubmixVoice(GetSubmixVoice(), GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputChannels);
		}
		virtual ~CSE() {};

		void PostUpdate()override {
			if (m_isAutoDelete && !GetIsPlaying()) {
				delete this;
			}
		}

		static IXAudio2SubmixVoice* GetSubmixVoice() {
			if (!m_SubmixVoice) {
				HRESULT hr;
				if (FAILED(hr = GetEngine().GetSoundEngine().GetIXAudio2()->CreateSubmixVoice(&m_SubmixVoice, GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputChannels, GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputSampleRate))) {
#ifndef DW_MASTER
					OutputDebugStringA("CreateSubmixVoice�Ɏ��s���܂����B\n");
#endif
					return nullptr;
				}
			}
			return m_SubmixVoice; 
		}

		void SetIsAutoDelete(bool isAutoDel){
			m_isAutoDelete = isAutoDel;
		}

	private:
		bool m_isAutoDelete = true;
		static IXAudio2SubmixVoice* m_SubmixVoice;		 
	}; 
	
	class CBGM : public CSound
	{
	public:
		CBGM(const wchar_t* fileName, bool isStreaming = true) : CSound::CSound(fileName, isStreaming) {
			SetUseSubmixVoice(GetSubmixVoice(), GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputChannels);
		}
		virtual ~CBGM() {};

		void PostUpdate()override {
			if (m_isAutoDelete && !GetIsPlaying()) {
				delete this;
			}
		}

		static IXAudio2SubmixVoice* GetSubmixVoice() {
			if (!m_SubmixVoice) {
				HRESULT hr;
				if (FAILED(hr = GetEngine().GetSoundEngine().GetIXAudio2()->CreateSubmixVoice(&m_SubmixVoice, GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputChannels, GetEngine().GetSoundEngine().GetSubmixVoiceDetails().InputSampleRate))) {
#ifndef DW_MASTER
					OutputDebugStringA("CreateSubmixVoice�Ɏ��s���܂����B\n");
#endif
					return nullptr;
				}
			}
			return m_SubmixVoice;
		}

		void SetIsAutoDelete(bool isAutoDel) {
			m_isAutoDelete = isAutoDel;
		}

	private:
		bool m_isAutoDelete = true;
		static IXAudio2SubmixVoice* m_SubmixVoice;
	};
}

}
}