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

		//ストリーミング再生の終了を待って自殺する
		/*void Delete() {
			Release();
			m_willDelete = true;
		}*/

		//いつもの
		void Update()override;

		//再生
		void Play(bool enable3D = false, bool isLoop = false);		
		//音を止める
		//※ストリーミング再生はすぐには停止しません
		void Stop();
		//一時停止
		void Pause();

		//3D再生用パラメーターを設定
		void SetPos(const CVector3& pos) { m_3DPos = pos; }
		void SetDistance(float dis) { m_distance = dis; }

		//設定を取得
		WAVSettingManager::WAVSetting* GetSetting() {
			return &m_setting;
		}
		void SetVolume(float vol) { m_setting.volume = vol; }
		void SetFrequencyRatio(float ratio) { m_setting.frequencyRatio = ratio; }
		void SetOutChannelVolume(int channel, float vol) { m_setting.outChannelVolume[channel] = vol; }

		//音データ自体への設定を取得
		WAVSettingManager::WAVSetting* GetDataSetting() {
			return m_pDataSetting;
		}
		void SetDataVolume(float vol) { m_pDataSetting->volume = vol; SetVolume(vol); }
		void SetDataFrequencyRatio(float ratio) { m_pDataSetting->frequencyRatio = ratio; SetDataFrequencyRatio(ratio); }
		void SetDataOutChannelVolume(int channel, float vol) { m_pDataSetting->outChannelVolume[channel] = vol; SetOutChannelVolume(channel, vol); }

		//再生中か取得(一時停止は考慮されない)
		bool GetIsPlaying()const { return m_sourceVoice; }
		bool GetIsPlayingAndNotPause()const { return !m_isPause && m_sourceVoice; }
		//一時停止中か取得
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

		bool m_isPause = false;

		WAVSettingManager::WAVSetting* m_pDataSetting = nullptr;
		WAVSettingManager::WAVSetting m_setting;

		std::vector<FLOAT32> m_defaultOutputMatrix;

		//3D再生用
		bool m_is3D = false;
		X3DAUDIO_EMITTER m_x3DEmitter = { 0 };
		X3DAUDIO_DSP_SETTINGS m_x3DDSPSettings = { 0 };
		std::vector<FLOAT32> m_matrixCoefficients;
		std::vector<FLOAT32> m_emitterAzimuths;
		CVector3 m_3DPos;
		float m_distance = 1000.0f;

		//ストリーミング用
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

		static const int STREAMING_BUFFER_SIZE = 65536;
		static const int MAX_BUFFER_COUNT = 3;
		static_assert((STREAMING_BUFFER_SIZE % 2048) == 0, "Streaming size must be 2K aligned to use for async I/O");

		BYTE m_buffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE] = {0};
	};
	
namespace Suicider {
	class CSE : public CSound
	{
	public:
		using CSound::CSound;
		virtual ~CSE() {};

		void PostUpdate()override {
			if (!GetIsPlaying()) { 
				delete this;
			}
		}
	}; 
	
	class CBGM : public CSound
	{
	public:
		CBGM(const wchar_t* fileName, bool isStreaming = true) : CSound::CSound(fileName, isStreaming) {}
		virtual ~CBGM() {};

		void PostUpdate()override {
			if (!GetIsPlaying()) {
				delete this;
			}
		}
	};
}

}
}