#include "DWstdafx.h"
#include "CFpsCounter.h"
#include <mmsystem.h>

namespace DemolisherWeapon {

namespace {
	wchar_t output[256];
}

CFpsCounter::CFpsCounter()
{
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nAfter);
	nBefore = nAfter;

	timeBeginPeriod(1);//Sleep�̐��x���グ��
}
CFpsCounter::~CFpsCounter()
{
	timeEndPeriod(1);//Sleep�̐��x��߂�
}

void CFpsCounter::Count() {
	QueryPerformanceCounter(&nAfter);
	m_frametimeSec = ((float)(nAfter.QuadPart - nBefore.QuadPart) / nFreq.QuadPart);//�P��:�b

	//�t���[�����[�g���
	if (m_useFpsLimiter && m_maxFrameTimeSec > m_frametimeSec) {
		//4msec�܂ł̓X���[�v
		while (m_maxFrameTimeSec - m_frametimeSec > 0.004f) {
			Sleep(1);
			QueryPerformanceCounter(&nAfter);
			m_frametimeSec = ((float)(nAfter.QuadPart - nBefore.QuadPart) / nFreq.QuadPart);//�P��:�b
		}
		//����ȉ��͐��m��
		while (m_maxFrameTimeSec > m_frametimeSec) {
			QueryPerformanceCounter(&nAfter);
			m_frametimeSec = ((float)(nAfter.QuadPart - nBefore.QuadPart) / nFreq.QuadPart);//�P��:�b
		}
	}

	nBefore = nAfter;//QueryPerformanceCounter(&nBefore);

	m_fps = 1.0f / m_frametimeSec;//�b�ԃt���[�����[�g

	m_runframecnt = m_frametimeSec / m_standardFrameTimeSec;//���������x����
}

void CFpsCounter::Draw() {
	if (!m_isDebugDraw) { return; }

	//�\��������̍X�V
	m_max = max(m_max, m_fps);
	m_min = min(m_min, m_fps);
	m_add += m_fps; m_avgcnt++; m_addTime += m_frametimeSec;
	if (m_addTime > 1.0f) {//1�b���Ƃɕ��ϒl�X�V
		m_avg = m_add / m_avgcnt;
		m_add = 0.0f; m_avgcnt = 0; m_addTime = 0.0f;
	}

	if (GetIsDebugInput() && GetAsyncKeyState(VK_NUMPAD5)) {
		m_max = 0.0f, m_min = 100000.0f;
	}

	//�e�L�X�g�쐬
	swprintf_s(output,
		L"%.1fFPS\n"
		"MAX:%.1f\n"
		"MIN:%.1f\n"
		"AVG:%.1f\n"
		"X%.4f\n"
		"GO_Num:%d\n"
		"ver.%ls",
		m_fps, m_max, m_min, m_avg, m_runframecnt, static_cast<int>(GetRegisterGameObjectNum()), DW_VER);

	//�\��
#ifndef DW_DX12
	GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, GetGraphicsEngine().GetCommonStates().NonPremultiplied());

	GetEngine().GetGraphicsEngine().GetSpriteFont()->DrawString(GetEngine().GetGraphicsEngine().GetSpriteBatchPMA(), output, { 0.0f,0.0f }, {1.0f,1.0f,1.0f,0.75f}, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), 0.4f); //DirectX::XMVectorScale(m_font->MeasureString(output), 0.0f));

	GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->End();
#else
	
	ID3D12DescriptorHeap* heaps[] = { GetGraphicsEngine().GetDirectXTK12DescriptorHeap()->Heap() };
	GetGraphicsEngine().GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);

	GetGraphicsEngine().GetSpriteBatchPMA()->Begin(GetGraphicsEngine().GetCommandList(), DirectX::SpriteSortMode::SpriteSortMode_Deferred);

	GetGraphicsEngine().GetSpriteFont()->DrawString(GetGraphicsEngine().GetSpriteBatchPMA(), output, { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,0.75f }, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), 0.4f);

	GetGraphicsEngine().GetSpriteBatchPMA()->End();//�`����s
	
#endif

	/*
	static bool isinied = false;
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
	if(!isinied){
		//���[�h
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

	//�`��
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