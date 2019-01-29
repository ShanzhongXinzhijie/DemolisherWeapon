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
		CSound(const wchar_t* fileName, bool isStreaming = false);
		virtual ~CSound();

		void Init(const wchar_t* fileName, bool isStreaming = false);
		void Release();

		void Update()override;
		void InUpdate(bool canStop = true);

		void Play(bool enable3D = false, bool isLoop = false);
		void Stop();
		void Pause();

		void SetPos(const CVector3& pos) { m_3DPos = pos; }
		void SetDistance(float dis) { m_distance = dis; }

		void SetVolume(float vol) { m_volume = vol; }

		bool GetIsPlaying()const { return !m_isPause && m_sourceVoice; }
		bool GetIsPausing()const { return m_isPause; }

	private:
		bool InitStreaming(const wchar_t* fileName);
		void ReleaseStreaming();
		void StreamingPlay(bool isLoop);
		void Streaming();
		
	private:
		bool m_isInit = false;

		WAVManager::WAVData* m_wav = nullptr;
		IXAudio2SourceVoice* m_sourceVoice = nullptr;

		bool m_isPause = false;

		float m_volume = 1.0f;

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
	/*class CSE : public CSound
	{
	public:

	};*/
}
	//効果音ファイル自体への設定
	//ピッチ・パン
}
}