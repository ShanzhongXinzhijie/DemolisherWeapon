#include "DWstdafx.h"
#include "WAVManager.h"
#include "DirectXTK/Audio/WAVFileReader.h"

namespace DemolisherWeapon {

	WAVManager::WAVManager()
	{
	}


	WAVManager::~WAVManager()
	{
	}

	WAVManager::WAVData* WAVManager::Load(const wchar_t* fileName) {

		int index = Util::MakeHash(fileName);

		//���ɓo�^����ĂȂ���?
		if (m_resourcesMap.count(index) > 0) {
			//�o�^����Ă���}�b�v����擾
			return &m_resourcesMap[index];
		}
		else {
			//�V�K�ǂݍ���
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
				sprintf_s(message, "DirectX::LoadWAVAudioFromFile()�Ɏ��s���܂���\nHRESULT:%x", hr );
				Error::Box(message);

				m_resourcesMap.erase(index);

				return nullptr;
			}
			else {
				return &m_resourcesMap[index];
			}
		}
	}

}