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

	//表示する情報の更新
	m_max = max(m_max, m_fps);
	m_min = min(m_min, m_fps);
	m_add += m_fps; m_avgcnt++; m_addTime += m_frametimeSec;
	if (m_addTime > 1.0f) {//1秒ごとに平均値更新
		m_avg = m_add / m_avgcnt;
		m_add = 0.0f; m_avgcnt = 0; m_addTime = 0.0f;
	}

	if (GetIsDebugInput() && GetAsyncKeyState(VK_NUMPAD5)) {
		m_max = 0.0f, m_min = 100000.0f;
	}

	//表示
#ifndef DW_DX12_TEMPORARY

	GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, GetGraphicsEngine().GetCommonStates().NonPremultiplied());

	wchar_t output[256];
	swprintf_s(output, 
		L"%.1fFPS\n"
		"MAX:%.1f\n"
		"MIN:%.1f\n"
		"AVG:%.1f\n"
		"X%.4f\n"
		"GO_Num:%d\n"
		"ver.%ls",
		m_fps, m_max, m_min, m_avg, m_runframecnt, static_cast<int>(GetRegisterGameObjectNum()), DW_VER);

	GetEngine().GetGraphicsEngine().GetSpriteFont()->DrawString(GetEngine().GetGraphicsEngine().GetSpriteBatch(), output, { 0.0f,0.0f }, {1.0f,1.0f,1.0f,0.75f}, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), 0.4f); //DirectX::XMVectorScale(m_font->MeasureString(output), 0.0f));

	GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();

#endif
}

}