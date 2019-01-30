#include "DWstdafx.h"
#include "CSound.h"

#include "DirectXTK/Src/PlatformHelpers.h"

namespace DemolisherWeapon {

namespace GameObj {

void CSound::Streaming() {

	bool exit = false;

	DWORD currentPosition = 0;
	DWORD currentDiskReadBuffer = 0;

	while (currentPosition < m_insAudioBytes) {

		if (m_threadBreak) { m_threadEnded = true; return; }

		DWORD cbValid = min(STREAMING_BUFFER_SIZE, m_insAudioBytes - currentPosition);
		m_ovlCurrentRequest.Offset = m_insStartAudio + currentPosition;

		bool wait = false;
		if (!ReadFile(m_async, m_buffers[currentDiskReadBuffer], STREAMING_BUFFER_SIZE, nullptr, &m_ovlCurrentRequest))
		{
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING)
			{
				wprintf(L"\nCouldn't start async read: error %#X\n", HRESULT_FROM_WIN32(error));
				exit = true;
				break;
			}
			wait = true;
		}

		currentPosition += cbValid;

		//
		// At this point the read is progressing in the background and we are free to do
		// other processing while we wait for it to finish. For the purposes of this sample,
		// however, we'll just go to sleep until the read is done.
		//
		if (wait) {
			WaitForSingleObject(m_ovlCurrentRequest.hEvent, INFINITE);
		}

		DWORD cb;
		BOOL result = GetOverlappedResultEx(m_async, &m_ovlCurrentRequest, &cb, 0, FALSE);
		if (!result)
		{
			wprintf(L"\nFailed waiting for async read: error %#X\n", HRESULT_FROM_WIN32(GetLastError()));
			exit = true;
			break;
		}

		//
		// Now that the event has been signaled, we know we have audio available. The next
		// question is whether our XAudio2 source voice has played enough data for us to give
		// it another buffer full of audio. We'd like to keep no more than MAX_BUFFER_COUNT - 1
		// buffers on the queue, so that one buffer is always free for disk I/O.
		//
		XAUDIO2_VOICE_STATE state;
		for (;; )
		{
			if (m_threadBreak) { m_threadEnded = true; return; }

			while (m_isLockSourceVoice.exchange(true)) {}//スピンロック
			m_sourceVoice->GetState(&state); 
			m_isLockSourceVoice = false;//スピンロック解除

			if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) { break; }

			WaitForSingleObject(m_voiceContext.hBufferEndEvent, INFINITE);
		}

		//
		// At this point we have a buffer full of audio and enough room to submit it, so
		// let's submit it and get another read request going.
		//
		XAUDIO2_BUFFER buf = { 0 };
		buf.AudioBytes = cbValid;
		buf.pAudioData = m_buffers[currentDiskReadBuffer];
		if (currentPosition >= m_insAudioBytes) {
			if (m_isStreamingLoop) {
				currentPosition = 0;//ループする(最初に戻る)
			}
			else {
				buf.Flags = XAUDIO2_END_OF_STREAM;//終わり
			}
		}
		while (m_isLockSourceVoice.exchange(true)) {}//スピンロック
		m_sourceVoice->SubmitSourceBuffer(&buf);
		m_isLockSourceVoice = false;//スピンロック解除

		currentDiskReadBuffer++;
		currentDiskReadBuffer %= MAX_BUFFER_COUNT;
	}

	//再生終わるのを待つ
	if (!exit)
	{
		XAUDIO2_VOICE_STATE state;
		for (; ; )
		{
			if (m_threadBreak) { m_threadEnded = true; return; }

			while (m_isLockSourceVoice.exchange(true)) {}//スピンロック
			m_sourceVoice->GetState(&state);
			m_isLockSourceVoice = false;//スピンロック解除

			if (!state.BuffersQueued) { break; }

			WaitForSingleObject(m_voiceContext.hBufferEndEvent, INFINITE);
		}
	}

	m_threadEnded = true;
}

bool CSound::InitStreaming(const wchar_t* fileName) {
	//ファイル開き
	DirectX::ScopedHandle hFile(DirectX::safe_handle(CreateFile2(fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		OPEN_EXISTING,
		nullptr)));

	if (!hFile) {
		return false;
	}

	// Get the file size
	FILE_STANDARD_INFO fileInfo;
	if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo))) {
		return false;
	}
	// File is too big for 32-bit allocation, so reject read
	if (fileInfo.EndOfFile.HighPart > 0) {
		return false;
	}
	// Need at least enough data to have a valid minimal WAV file
	if (fileInfo.EndOfFile.LowPart < (sizeof(uint32_t) * 2 * 2 + sizeof(DWORD) + sizeof(WAVEFORMAT))) {
		return false;
	}

	//読み込みだ！
	const uint32_t FOURCC_RIFF_TAG = 'FFIR';
	const uint32_t FOURCC_FORMAT_TAG = ' tmf';
	const uint32_t FOURCC_DATA_TAG = 'atad';
	const uint32_t FOURCC_WAVE_FILE_TAG = 'EVAW';
	const uint32_t FOURCC_XWMA_FILE_TAG = 'AMWX';
	const uint32_t FOURCC_DLS_SAMPLE = 'pmsw';
	const uint32_t FOURCC_MIDI_SAMPLE = 'lpms';
	const uint32_t FOURCC_XWMA_DPDS = 'sdpd';
	const uint32_t FOURCC_XMA_SEEK = 'kees';

	bool isLoadedRIFF	= false;
	bool isLoadedFormat = false;
	bool isLoadedData	= false;

	uint32_t data[2] = { 0 };
	DWORD readed = 0;
	bool dpds, seek;
	dpds = seek = false;
	while (1) {
		bool isRIFF = false;
		bool notMovePointer = true;

		DWORD bytesRead = 0;

		if (!ReadFile(hFile.get(),
			data,
			sizeof(uint32_t) * 2,
			&bytesRead,
			nullptr
		))
		{
			return false;
		}

		//tag判別
		if (readed == 0 && data[0] != FOURCC_RIFF_TAG) {
			//最初はRIFF
			return false;
		}
		//RIFF&WAVE
		if (data[0] == FOURCC_RIFF_TAG) {
			isRIFF = true;
			if (data[1] < 4) { return false; }

			uint32_t riff = 0;
			DWORD bytesRead2 = 0;
			if (!ReadFile(hFile.get(),
				&riff,
				sizeof(uint32_t),
				&bytesRead2,
				nullptr
			))
			{
				return false;
			}
			bytesRead += bytesRead2;
			notMovePointer = false;
			if (riff != FOURCC_WAVE_FILE_TAG && riff != FOURCC_XWMA_FILE_TAG) {
				//WAVEじゃない
				return false;
			}

			isLoadedRIFF = true;
		}
		//FORMAT
		if (data[0] == FOURCC_FORMAT_TAG) {
			if (data[1] < sizeof(PCMWAVEFORMAT)) {
				return false;
			}

			WAVEFORMAT fmt = { 0 };
			DWORD bytesRead2 = 0;
			if (!ReadFile(hFile.get(),
				&fmt,
				sizeof(WAVEFORMAT),
				&bytesRead2,
				nullptr
			))
			{
				return false;
			}
			bytesRead += bytesRead2;
			notMovePointer = false;

			// Validate WAVEFORMAT (focused on chunk size and format tag, not other data that XAUDIO2 will validate)
			switch (fmt.wFormatTag)
			{
			case WAVE_FORMAT_PCM:
			case WAVE_FORMAT_IEEE_FLOAT:
				// Can be a PCMWAVEFORMAT (8 bytes) or WAVEFORMATEX (10 bytes)
				// We validiated chunk as at least sizeof(PCMWAVEFORMAT) above
				break;

			default:
			{
				if (data[1] < sizeof(WAVEFORMATEX))
				{
					return  false;
				}

				//ポインタ戻す
				SetFilePointer(hFile.get(), -(LONG)bytesRead2, NULL, FILE_CURRENT);
				bytesRead -= bytesRead2;
				bytesRead2 = 0;
				notMovePointer = true;

				WAVEFORMATEX fmtx = { 0 };
				if (!ReadFile(hFile.get(),
					&fmtx,
					sizeof(WAVEFORMATEX),
					&bytesRead2,
					nullptr
				))
				{
					return false;
				}
				bytesRead += bytesRead2;
				notMovePointer = false;

				if (data[1] < (sizeof(WAVEFORMATEX) + fmtx.cbSize))
				{
					return false;
				}

				switch (fmtx.wFormatTag)
				{
				case WAVE_FORMAT_WMAUDIO2:
				case WAVE_FORMAT_WMAUDIO3:
					dpds = true;
					break;

				case  0x166 /*WAVE_FORMAT_XMA2*/: // XMA2 is supported by Xbox One
					if ((data[1] < 52 /*sizeof(XMA2WAVEFORMATEX)*/) || (fmtx.cbSize < 34 /*( sizeof(XMA2WAVEFORMATEX) - sizeof(WAVEFORMATEX) )*/))
					{
						return  false;
					}
					seek = true;
					break;

				case WAVE_FORMAT_ADPCM:
					if ((data[1] < (sizeof(WAVEFORMATEX) + 32)) || (fmtx.cbSize < 32 /*MSADPCM_FORMAT_EXTRA_BYTES*/))
					{
						return  false;
					}
					break;

				case WAVE_FORMAT_EXTENSIBLE:
					if ((data[1] < sizeof(WAVEFORMATEXTENSIBLE)) || (fmtx.cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))))
					{
						return  false;
					}
					else
					{
						static const GUID s_wfexBase = { 0x00000000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };

						//ポインタ戻す
						SetFilePointer(hFile.get(), -(LONG)bytesRead2, NULL, FILE_CURRENT);
						bytesRead -= bytesRead2;
						bytesRead2 = 0;
						notMovePointer = true;

						WAVEFORMATEXTENSIBLE fmtex = { 0 };
						if (!ReadFile(hFile.get(),
							&fmtex,
							sizeof(WAVEFORMATEXTENSIBLE),
							&bytesRead2,
							nullptr
						))
						{
							return false;
						}
						bytesRead += bytesRead2;
						notMovePointer = false;

						if (memcmp(reinterpret_cast<const BYTE*>(&fmtex.SubFormat) + sizeof(DWORD),
							reinterpret_cast<const BYTE*>(&s_wfexBase) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD)) != 0)
						{
							return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
						}

						switch (fmtex.SubFormat.Data1)
						{
						case WAVE_FORMAT_PCM:
						case WAVE_FORMAT_IEEE_FLOAT:
							break;

							// MS-ADPCM and XMA2 are not supported as WAVEFORMATEXTENSIBLE

						case WAVE_FORMAT_WMAUDIO2:
						case WAVE_FORMAT_WMAUDIO3:
							dpds = true;
							break;

						default:
							return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
						}

					}
					break;

				default:
					return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
				}
			}
			}

			//ポインタ戻す
			DWORD pointerOffset = bytesRead2;
			SetFilePointer(hFile.get(), -(LONG)bytesRead2, NULL, FILE_CURRENT);
			bytesRead -= bytesRead2;
			bytesRead2 = 0;
			notMovePointer = true;

			DWORD readSize = min(data[1], sizeof(WAVEFORMATEX));

			if (!ReadFile(hFile.get(),
				&m_insWfx,
				readSize,
				&bytesRead2,
				nullptr
			))
			{
				return false;
			}
			bytesRead += bytesRead2;
			notMovePointer = false;

			if (bytesRead2 < pointerOffset) {
				SetFilePointer(hFile.get(), pointerOffset - bytesRead2, NULL, FILE_CURRENT);
				bytesRead += pointerOffset - bytesRead2;
			}

			isLoadedFormat = true;
		}
		//DATA
		if (data[0] == FOURCC_DATA_TAG) {
			if (!data[1]) { return false; }
			m_insStartAudio = readed + bytesRead;//データ部分始まり
			m_insAudioBytes = data[1];//データ部分サイズ

			isLoadedData = true;
		}

		//ポインタ進める
		if (!isRIFF && bytesRead < data[1] + sizeof(uint32_t) * 2) {//notMovePointer) {
			SetFilePointer(hFile.get(), (data[1] + sizeof(uint32_t) * 2) - bytesRead, NULL, FILE_CURRENT);
			bytesRead += (data[1] + sizeof(uint32_t) * 2) - bytesRead;
		}

		//データ足りなくなったら出る
		readed += bytesRead;
		if (readed + sizeof(uint32_t) * 2 > fileInfo.EndOfFile.LowPart) {
			if (!isLoadedRIFF || !isLoadedFormat || !isLoadedData) {
				//情報足りてないとエラー
				return false;
			}
			break;
		}
	}

	if ((STREAMING_BUFFER_SIZE % m_insWfx.nBlockAlign) != 0) {
		//
		// non-PCM data will fail here. ADPCM requires a more complicated streaming mechanism to deal with submission in audio frames that do
		// not necessarily align to the 2K async boundary.
		//		
#ifndef DW_MASTER
		char message[256];
		sprintf_s(message, "\nStreaming buffer size (%u) is not aligned with sample block requirements (%u)\n", STREAMING_BUFFER_SIZE, m_insWfx.nBlockAlign);
		OutputDebugStringA(message);
#endif
		return false;
	}

	m_fileName = fileName;

	return (dpds || seek) ? false : true;
}

void CSound::StreamingPlay(bool isLoop) {
	if (m_sourceVoice || !m_isStreaming)return;

	for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
		for (int i2 = 0; i2 < STREAMING_BUFFER_SIZE; i2++) {
			m_buffers[i][i2] = 0;
		}
	}

	m_isStreamingLoop = isLoop;

	//ストリーミング用読み込みハンドル作成
	CREATEFILE2_EXTENDED_PARAMETERS params2 = { sizeof(CREATEFILE2_EXTENDED_PARAMETERS), 0 };
	params2.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	params2.dwFileFlags = FILE_FLAG_OVERLAPPED;// | FILE_FLAG_NO_BUFFERING;
	m_async = CreateFile2(m_fileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		OPEN_EXISTING,
		&params2);

	m_ovlCurrentRequest = { 0 };
	m_ovlCurrentRequest.hEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);

	XAUDIO2_SEND_DESCRIPTOR sendDescriptors[1];
	sendDescriptors[0].Flags = 0;
	sendDescriptors[0].pOutputVoice = GetEngine().GetSoundEngine().GetSubmixVoice();
	const XAUDIO2_VOICE_SENDS sendList = { 1, sendDescriptors };

	HRESULT hr = GetEngine().GetSoundEngine().GetIXAudio2()->CreateSourceVoice(&m_sourceVoice, &m_insWfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_voiceContext, &sendList);
	if (FAILED(hr)) {
		char message[256];
		sprintf_s(message, "CreateSourceVoice()に失敗しました\nHRESULT:%x", hr);
		Error::Box(message);
		m_sourceVoice = nullptr;
		return;
	}

	//デフォ行列保存
	m_sourceVoice->GetOutputMatrix(GetEngine().GetSoundEngine().GetSubmixVoice(), m_x3DDSPSettings.SrcChannelCount, m_x3DDSPSettings.DstChannelCount, m_defaultOutputMatrix.data());

	InUpdate(false);

	m_sourceVoice->Start();

	m_threadEnded = false; 
	m_threadBreak = false;
	m_thread = std::thread([&] {Streaming(); });
}

void CSound::ReleaseStreaming() {

	if (m_thread.joinable()) {
		m_threadBreak = true;
		return;
	}

	if (m_async != INVALID_HANDLE_VALUE)
	{
		if (m_ovlCurrentRequest.hEvent)
		{
			DWORD bytes;
			(void)GetOverlappedResultEx(m_async, &m_ovlCurrentRequest, &bytes, INFINITE, FALSE);
		}

		CloseHandle(m_async);
		m_async = INVALID_HANDLE_VALUE;
	}
	if (m_ovlCurrentRequest.hEvent) {
		CloseHandle(m_ovlCurrentRequest.hEvent);
		m_ovlCurrentRequest.hEvent = 0;
	}
}

}
}