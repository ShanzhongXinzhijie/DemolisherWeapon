#include "DWstdafx.h"
#include "WAVManager.h"

namespace DemolisherWeapon {

	WAVManager::WAVData* WAVManager::Load(const wchar_t* fileName) {

		int index = Util::MakeHash(fileName);

		//既に登録されてないか?
		if (m_resourcesMap.count(index) > 0) {
			//登録されてたらマップから取得
			return &m_resourcesMap[index];
		}
		else {
			//新規読み込み
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
				sprintf_s(message, "DirectX::LoadWAVAudioFromFile()に失敗しました\nHRESULT:%x", hr );
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

		//既に登録されてないか?
		if (m_settingMap.count(index) > 0) {
			//登録されてたらマップから取得
			return &m_settingMap[index];
		}
		else {
			//新規作成
			m_settingMap.emplace(index, WAVSetting());
			return &m_settingMap[index];
		}
	}
}