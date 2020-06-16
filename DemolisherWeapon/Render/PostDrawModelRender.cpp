#include "DWstdafx.h"
#include "PostDrawModelRender.h"

namespace DemolisherWeapon {

	void PostDrawModelRender::Init() {
		D3D11_BLEND_DESC blendDesc;
		D3D11_DEPTH_STENCIL_DESC depthDesc;

		//�A���t�@�u�����h�X�e�[�g(��Z�ς�)
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		GetGraphicsEngine().GetD3DDevice()->CreateBlendState(&blendDesc, m_alphaBlendState.ReleaseAndGetAddressOf());

		//���Z�u�����h�X�e�[�g
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_GREEN;
		GetGraphicsEngine().GetD3DDevice()->CreateBlendState(&blendDesc, m_addBlendState.ReleaseAndGetAddressOf());
	
		//�f�v�X�X�e���V���X�e�[�g		
		ZeroMemory(&depthDesc, sizeof(depthDesc));
		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthDesc.StencilEnable = false;
		depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		GetGraphicsEngine().GetD3DDevice()->CreateDepthStencilState(&depthDesc, &m_depthStencilState);
	}

	void PostDrawModelRender::Render() {
#ifndef DW_MASTER
		if (!GetMainCamera()) {
			MessageBox(NULL, "�J�������ݒ肳��Ă��܂���!!", "Error", MB_OK);
			std::abort();
		}
#endif
		//GPU�C�x���g�̊J�n
		GetGraphicsEngine().BeginGPUEvent(L"PostDrawModelRender");

		//�u�����h�X�e�[�g�ύX
		ID3D11BlendState* oldBlendState = nullptr; FLOAT oldf[4]; UINT olduint;
		GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldf, &olduint);
		
		//�`�����ŏI�����_�[�^�[�Q�b�g�ɂ���
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
			1,
			&GetGraphicsEngine().GetFRT().GetRTV(),
			GetGraphicsEngine().GetGBufferRender().GetDSV()
		);
		
		//���f���`��
		for (int i = 0; i < DRAW_PRIORITY_MAX; i++) {
			//���f���`��(���Z�u�����h)		
			for (auto& model : m_drawModelList_Add[i]) {
				model.first->Draw(model.second, 1, m_addBlendState.Get(), m_depthStencilState.Get());
			}
			//���f���`��(�A���t�@�u�����h)
			for (auto& model : m_drawModelList_Alpha[i]) {
				model.first->Draw(model.second, 1, m_alphaBlendState.Get(), m_depthStencilState.Get());
			}
		}

		//���X�^���C�U�[�X�e�[�g�߂�
		GetGraphicsEngine().ResetRasterizerState();

		//�����_�[�^�[�Q�b�g����
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//�u�����h�X�e�[�g�߂�
		if (oldBlendState) {
			GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldf, olduint);
			oldBlendState->Release();
		}		

		//GPU�C�x���g�̏I��
		GetGraphicsEngine().EndGPUEvent();
	}

	void PostDrawModelRender::PostRender() {
		//���f���o�^�̉���
		for (auto& list : m_drawModelList_Alpha) {
			list.clear();
		}
		for (auto& list : m_drawModelList_Add) {
			list.clear();
		}
	}
}