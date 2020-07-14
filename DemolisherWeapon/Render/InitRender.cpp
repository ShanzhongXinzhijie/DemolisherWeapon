#include "DWstdafx.h"
#include "InitRender.h"

namespace DemolisherWeapon {
	void InitRender::Render() {
#ifndef DW_DX12_TEMPORARY
		//�u�����h�X�e�[�g������
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(GetGraphicsEngine().GetCommonStates().NonPremultiplied(), nullptr, 0xFFFFFFFF);
#endif
		//�X�N���[���ԍ���ݒ�
		GetGraphicsEngine().SetTargetScreenNum(-1);

		/*
			//�O�t���[���̕`�抮����҂�
			if (!WaitForPreviousFrame()) {
				return;
			}

			//���ꂩ��g���R�}���h���X�g�E�A���P�[�^�����Z�b�g���Ďg�p�\��
			if (FAILED(m_commandAllocator[m_currentBackBufferIndex]->Reset())) {
				return;
			}
			if (FAILED(m_commandList->Reset(m_commandAllocator[m_currentBackBufferIndex].Get(), nullptr))) {
				return;
			}

			//���\�[�X�o���A��ݒ�
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

			//�����_�[�^�[�Q�b�g��ݒ�
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			rtvHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;
			m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		*/

		//�o�b�N�o�b�t�@���N���A
		GetGraphicsEngine().ClearBackBuffer();

		//3D�p�̃r���[�|�[�g�ɂ���
		GetGraphicsEngine().SetViewport(0.0f, 0.0f, GetGraphicsEngine().Get3DFrameBuffer_W(), GetGraphicsEngine().Get3DFrameBuffer_H());
	}
}