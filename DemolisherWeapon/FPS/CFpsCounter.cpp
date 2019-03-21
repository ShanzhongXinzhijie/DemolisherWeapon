#include "DWstdafx.h"
#include "CFpsCounter.h"
#include <mmsystem.h>

namespace DemolisherWeapon {

CFpsCounter::CFpsCounter()
{
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nAfter);
	nBefore = nAfter;

	timeBeginPeriod(1);//Sleepの精度を上げる
}
CFpsCounter::~CFpsCounter()
{
	timeEndPeriod(1);//Sleepの精度を戻す
}

void CFpsCounter::Count() {
	QueryPerformanceCounter(&nAfter);
	m_frametimeSec = ((float)(nAfter.QuadPart - nBefore.QuadPart) / nFreq.QuadPart);//単位:秒

	//フレームレート上限
	if (m_useFpsLimiter && m_maxFrameTimeSec > m_frametimeSec) {
		//4msecまではスリープ
		while (m_maxFrameTimeSec - m_frametimeSec > 0.004f) {
			Sleep(1);

			QueryPerformanceCounter(&nAfter);
			m_frametimeSec = ((float)(nAfter.QuadPart - nBefore.QuadPart) / nFreq.QuadPart);//単位:秒
		}
		//それ以下は正確に
		while (m_maxFrameTimeSec > m_frametimeSec) {
			QueryPerformanceCounter(&nAfter);
			m_frametimeSec = ((float)(nAfter.QuadPart - nBefore.QuadPart) / nFreq.QuadPart);//単位:秒
		}
	}

	nBefore = nAfter;//QueryPerformanceCounter(&nBefore);

	m_fps = 1.0f / m_frametimeSec;//秒間フレームレート

	m_runframecnt = m_frametimeSec / m_standardFrameTimeSec;//処理落ち度合い
}

void CFpsCounter::Draw() {
	if (!m_isDebugDraw) { return; }

	static float s_max = 0.0f, s_min = 100000.0f, s_avg = 0.0f, s_add = 0.0f;
	static int s_avgcnt = 0;
	s_max = max(s_max, m_fps);
	s_min = min(s_min, m_fps);
	s_add += m_fps; s_avgcnt++;
	if (s_avgcnt >= 60) {
		s_avg = s_add / s_avgcnt;
		s_add = 0.0f; s_avgcnt = 0;
	}

	if (GetIsDebugInput() && GetAsyncKeyState(VK_NUMPAD5)) {
		s_max = 0.0f, s_min = 100000.0f;
	}

	GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin();

	wchar_t output[256];
	swprintf_s(output, L"%.1fFPS\nMAX:%.1f\nMIN:%.1f\nAVG:%.1f\nX%.4f\nver.%ls", m_fps, s_max, s_min, s_avg, m_runframecnt, DW_VER);

	GetEngine().GetGraphicsEngine().GetSpriteFont()->DrawString(GetEngine().GetGraphicsEngine().GetSpriteBatch(), output, { 0.0f,0.0f }, DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f),0.5f); //DirectX::XMVectorScale(m_font->MeasureString(output), 0.0f));

	GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();
}

}