#include "DWstdafx.h"
#include "CFpsCounter.h"
#include <mmsystem.h>

namespace DemolisherWeapon {

namespace {
	wchar_t output[256];

	constexpr wchar_t API_NAME_DX11[] = L"DX11";
	constexpr wchar_t API_NAME_DX12[] = L"DX12";
}

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

	//テキスト作成
	const wchar_t* apiName = nullptr;
	if (GetGraphicsAPI() == enDirectX11) { apiName = API_NAME_DX11; }
	if (GetGraphicsAPI() == enDirectX12) { apiName = API_NAME_DX12; }
	swprintf_s(output,
		L"%.1fFPS\n"
		"MAX:%.1f\n"
		"MIN:%.1f\n"
		"AVG:%.1f\n"
		"X%.4f\n"
		"GO_Num:%d\n"
		"ver.%ls\n"
		"API:%ls",
		m_fps, m_max, m_min, m_avg, m_runframecnt, static_cast<int>(GetRegisterGameObjectNum()), DW_VER, apiName);

	//表示
#ifndef DW_DX12
	GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, GetGraphicsEngine().GetCommonStates().NonPremultiplied());

	GetEngine().GetGraphicsEngine().GetSpriteFont()->DrawString(GetEngine().GetGraphicsEngine().GetSpriteBatchPMA(), output, { 0.0f,0.0f }, {1.0f,1.0f,1.0f,0.75f}, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), 0.4f); //DirectX::XMVectorScale(m_font->MeasureString(output), 0.0f));

	GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->End();
#else	
	ID3D12DescriptorHeap* heaps[] = { GetGraphicsEngine().GetDirectXTK12DescriptorHeap() };
	GetGraphicsEngine().GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);

	GetGraphicsEngine().GetSpriteBatchPMA()->Begin(GetGraphicsEngine().GetCommandList(), DirectX::SpriteSortMode::SpriteSortMode_Deferred);

	GetGraphicsEngine().GetSpriteFont()->DrawString(GetGraphicsEngine().GetSpriteBatchPMA(), output, { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,0.75f }, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), 0.4f);

	GetGraphicsEngine().GetSpriteBatchPMA()->End();//描画実行	
#endif

	/*
	static bool isinied = false;
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
	if(!isinied){
		//ロード
		DirectX::ResourceUploadBatch resourceUpload(GetGraphicsEngine().GetD3D12Device());

		resourceUpload.Begin();

		DirectX::ThrowIfFailed(
			CreateWICTextureFromFile(GetGraphicsEngine().GetD3D12Device(), resourceUpload, L"BButton.png",
				m_texture.ReleaseAndGetAddressOf()));

		DirectX::CreateShaderResourceView(GetGraphicsEngine().GetD3D12Device(), m_texture.Get(), GetGraphicsEngine().GetDirectXTK12DescriptorHeap()->GetCpuHandle(GraphicsEngine::Descriptors::Sprite));

		auto uploadResourcesFinished = resourceUpload.End(GetGraphicsEngine().GetXTK12CommandQueue());

		uploadResourcesFinished.wait();

		isinied = true;
	}

	//描画
	ID3D12DescriptorHeap* heaps[] = { GetGraphicsEngine().GetDirectXTK12DescriptorHeap()->Heap(), GetGraphicsEngine().GetCommonStates().Heap() };
	GetGraphicsEngine().GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);

	GetGraphicsEngine().GetSpriteBatch()->Begin(GetGraphicsEngine().GetCommandList());

	GetGraphicsEngine().GetSpriteBatch()->Draw(
		GetGraphicsEngine().GetDirectXTK12DescriptorHeap()->GetGpuHandle(GraphicsEngine::Descriptors::Sprite),
		DirectX::GetTextureSize(m_texture.Get()),
		DirectX::Colors::White
	);

	GetGraphicsEngine().GetSpriteBatch()->End();
	*/
}

}