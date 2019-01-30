#pragma once

namespace DemolisherWeapon {
	static const int MAX_CHANNEL = 8;

	class WAVManager
	{
	public:
		struct WAVData {
			std::unique_ptr<uint8_t[]> wavData;
			const WAVEFORMATEX* wfx;
			const uint8_t* startAudio;
			uint32_t audioBytes;
		};

		WAVData* Load(const wchar_t* fileName);		

	private:
		std::unordered_map<int, WAVData> m_resourcesMap;
	};

	//音声データ自体への設定を扱うクラス
	class WAVSettingManager
	{
	public:
		struct WAVSetting {
			float volume = 1.0f;
			float outChannelVolume[MAX_CHANNEL] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
			float frequencyRatio = 1.0f;
		};

		WAVSetting* Load(const wchar_t* fileName);

	private:
		std::unordered_map<int, WAVSetting> m_settingMap;
	};

}
