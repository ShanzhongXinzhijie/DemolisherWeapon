#pragma once

namespace DemolisherWeapon {

	class WAVManager
	{
	public:
		struct WAVData {
			std::unique_ptr<uint8_t[]> wavData;
			const WAVEFORMATEX* wfx;
			const uint8_t* startAudio;
			uint32_t audioBytes;
		};

		WAVManager();
		~WAVManager();

		WAVData* Load(const wchar_t* fileName);		

	private:
		std::unordered_map<int, WAVData> m_resourcesMap;
	};

}
