#include "DWstdafx.h"
#include "WAVManager.h"

namespace DemolisherWeapon {

	WAVManager::WAVData* WAVManager::Load(const wchar_t* fileName) {

		int index = Util::MakeHash(fileName);

		//Šù‚É“o˜^‚³‚ê‚Ä‚È‚¢‚©?
		if (m_resourcesMap.count(index) > 0) {
			//“o˜^‚³‚ê‚Ä‚½‚çƒ}ƒbƒv‚©‚çæ“¾
			return &m_resourcesMap[index];
		}
		else {
			//V‹K“Ç‚İ‚İ
			m_resourcesMap.emplace(index, WAVData());
			HRESULT hr = DirectX::LoadWAVAudioFromFile(
				fileName,
				m_resourcesMap[index].wavData,
				&m_resourcesMap[index].wfx,
				&m_resourcesMap[index].startAudio,
				&m_resourcesMap[index].audioBytes
			);
			if (FAILED(hr)) {
				char message[256];
				sprintf_s(message, "DirectX::LoadWAVAudioFromFile()‚É¸”s‚µ‚Ü‚µ‚½\nHRESULT:%x", hr );
				Error::Box(message);

				m_resourcesMap.erase(index);

				return nullptr;
			}
			else {
				return &m_resourcesMap[index];
			}
		}
	}

	WAVSettingManager::WAVSetting* WAVSettingManager::Load(const wchar_t* fileName) {

		int index = Util::MakeHash(fileName);

		//Šù‚É“o˜^‚³‚ê‚Ä‚È‚¢‚©?
		if (m_settingMap.count(index) > 0) {
			//“o˜^‚³‚ê‚Ä‚½‚çƒ}ƒbƒv‚©‚çæ“¾
			return &m_settingMap[index];
		}
		else {
			//V‹Kì¬
			m_settingMap.emplace(index, WAVSetting());
			return &m_settingMap[index];
		}
	}
}