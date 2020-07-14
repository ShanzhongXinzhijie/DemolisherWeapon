#pragma once
#include "GraphicsAPI/IGraphicsAPI.h"

namespace DemolisherWeapon {

	class DX11Test : public IGraphicsAPI
	{
	public:
		bool Init(HWND hWnd, const InitEngineParameter& initParam)override;
		void Release()override;

		/// <summary>
		/// �t���[���o�b�t�@�T�C�Y�̕ύX(�Đݒ�)
		/// </summary>
		void ChangeFrameBufferSize()override;

		/// <summary>
		/// D3D11�f�o�C�X���擾
		/// </summary>
		ID3D11Device* GetD3DDevice()
		{
			return m_pd3dDevice;
		}

		/// <summary>
		/// D3D11�f�o�C�X�R���e�L�X�g���擾
		/// </summary>
		ID3D11DeviceContext* GetD3DDeviceContext()
		{
			return m_pd3dDeviceContext;
		}

		/// <summary>
		/// ���X�^���C�U�[�X�e�[�g�����Z�b�g
		/// </summary>
		void ResetRasterizerState() {
			m_pd3dDeviceContext->RSSetState(m_rasterizerState);
		}

		/// <summary>
		/// �o�b�N�o�b�t�@�̃N���A
		/// </summary>
		void ClearBackBuffer()override
		{
			//�o�b�N�o�b�t�@���D�F�œh��Ԃ��B
			float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
			m_pd3dDeviceContext->ClearRenderTargetView(m_backBuffer, ClearColor);
			//�f�v�X�̃N���A
			m_pd3dDeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}

		void SetBackBufferToRenderTarget();
		void SwapBackBuffer();

		/// <summary>
		/// �r���[�|�[�g�̐ݒ�
		/// </summary>
		void SetViewport(float topLeftX, float topLeftY, float width, float height)override
		{
			m_viewport.Width = width;
			m_viewport.Height = height;
			m_viewport.TopLeftX = topLeftX;
			m_viewport.TopLeftY = topLeftY;
			m_viewport.MinDepth = 0.0f;
			m_viewport.MaxDepth = 1.0f;
			m_pd3dDeviceContext->RSSetViewports(1, &m_viewport);
		}

	private:
		ID3D11Device* m_pd3dDevice = nullptr;					//D3D11�f�o�C�X�B
		ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;		//D3D11�f�o�C�X�R���e�L�X�g�B
		IDXGISwapChain* m_pSwapChain = nullptr;					//�X���b�v�`�F�C���B
		ID3D11RenderTargetView* m_backBuffer = nullptr;			//�o�b�N�o�b�t�@�B
		ID3D11RasterizerState* m_rasterizerState = nullptr;		//���X�^���C�U�X�e�[�g�B
		ID3D11Texture2D* m_depthStencil = nullptr;				//�f�v�X�X�e���V���B
		ID3D11DepthStencilView* m_depthStencilView = nullptr;	//�f�v�X�X�e���V���r���[�B
		ID3D11DepthStencilState* m_depthStencilState = nullptr;
		
		D3D11_VIEWPORT m_viewport;//�r���[�|�[�g
	};

}

