#pragma once

namespace DemolisherWeapon {

class SoundEngine
{
public:
	SoundEngine();
	~SoundEngine();

	void Init();
	void Release();

	IXAudio2* GetIXAudio2(){ return m_pXAudio2; }

private:
	IXAudio2* m_pXAudio2 = nullptr;
	IXAudio2MasteringVoice* m_pMasterVoice = nullptr;
};

}