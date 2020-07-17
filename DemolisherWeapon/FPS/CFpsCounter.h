#pragma once

namespace DemolisherWeapon {

class CFpsCounter
{
public:
	CFpsCounter();
	~CFpsCounter();

	//初期化
	void Init(int maxfps, int stdfps, bool useFpsLimiter) {
		SetUseFpsLimiter(useFpsLimiter,maxfps);
		SetStandardFrameRate(stdfps);
	}

	//フレームレート計測
	void Count();

	//フレームレート描画
	void Draw();

	//前回フレームの実行にかかった時間
	float GetFrameTimeSec() const { return m_frametimeSec; };
	//実行した動作フレーム数を取得
	float GetRunFrameCnt() const { return m_runframecnt; };

	//fps上限を設定
	void SetUseFpsLimiter(bool use, int maxfps = -1) { 
		m_useFpsLimiter = use;
		if (maxfps >= 1) {
			m_maxfps = maxfps; 
			m_maxFrameTimeSec = 1.0f / m_maxfps; 
		} 
	}
	//動作基準FPSを設定
	void SetStandardFrameRate(int fps) {
		m_standardfps = fps; 
		m_standardFrameTimeSec = 1.0f / m_standardfps;
	}

	//動作基準FPSを取得
	int GetStandardFrameRate()const { return m_standardfps; }

	//デバッグ情報を描画するか設定
	void SetIsDebugDraw(bool enable) { m_isDebugDraw = enable; }
	bool GetIsDebugDraw()const { return m_isDebugDraw; }

private:
	LARGE_INTEGER nFreq, nBefore, nAfter;
	float m_frametimeSec = 0.0f, m_fps = 0.0f, m_runframecnt = 0.0f;

	bool m_useFpsLimiter = true;
	int m_maxfps = 60;
	float m_maxFrameTimeSec = 1.0f / m_maxfps;

	int m_standardfps = 60;
	float m_standardFrameTimeSec = 1.0f / m_standardfps;

#ifndef DW_MASTER
	bool m_isDebugDraw = true;
#else
	bool m_isDebugDraw = false;
#endif

	//fps表示用
	float m_max = 0.0f, m_min = 100000.0f, m_avg = 0.0f, m_add = 0.0f, m_addTime = 0.0f;
	int m_avgcnt = 0;
};

}