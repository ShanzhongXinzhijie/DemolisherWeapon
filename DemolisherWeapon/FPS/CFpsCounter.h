#pragma once

namespace DemolisherWeapon {

class CFpsCounter
{
public:
	CFpsCounter();
	~CFpsCounter();

	void Init(int maxfps, int stdfps) {
		m_maxfps = maxfps; m_maxFrameTimeSec = 1.0f / m_maxfps;
		m_standardfps = stdfps; m_standardFrameTimeSec = 1.0f / m_standardfps;
	}

	void Count();

	void Draw();

	float GetFrameTimeSec() const { return m_frametimeSec; };
	float GetRunFrameCnt() const { return m_runframecnt; };

	//fpsãŒÀ‚ğİ’è
	void SetUseFpsLimiter(bool use, int maxfps = -1) { m_useFpsLimiter = use; if (maxfps >= 1) { m_maxfps = maxfps; m_maxFrameTimeSec = 1.0f / m_maxfps; } }

	//“®ìŠî€FPS‚ğæ“¾
	int GetStandardFrameRate()const { return m_standardfps; }

private:
	LARGE_INTEGER nFreq, nBefore, nAfter;
	float m_frametimeSec = 0.0f, m_fps = 0.0f, m_runframecnt = 0.0f;

	bool m_useFpsLimiter = true;
	int m_maxfps = 60;
	float m_maxFrameTimeSec = 1.0f / m_maxfps;

	int m_standardfps = 60;
	float m_standardFrameTimeSec = 1.0f / m_standardfps;
};

}