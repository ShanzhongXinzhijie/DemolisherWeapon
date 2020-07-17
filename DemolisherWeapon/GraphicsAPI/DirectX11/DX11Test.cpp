#include "DWstdafx.h"
#include "DX11Test.h"

namespace DemolisherWeapon {

	namespace {
		template<class T>
		void RELEASE(T*& t) {
			if (t) {
				t->Release();
				t = nullptr;
			}
		}
	}

	bool DX11Test::Init(HWND hWnd, const InitEngineParameter& initParam) {
		//���t���b�V�����[�g���擾
		auto refleshRate = Util::GetRefreshRate(hWnd);

		//�X���b�v�`�F�C�����쐬���邽�߂̏���ݒ肷��B
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;									//�X���b�v�`�F�C���̃o�b�t�@���B�ʏ�͂P�B
		sd.BufferDesc.Width = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W());	//�t���[���o�b�t�@�̕��B
		sd.BufferDesc.Height = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H());	//�t���[���o�b�t�@�̍����B
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//�t���[���o�b�t�@�̃t�H�[�}�b�g�BR8G8B8A8��32bit�B
		sd.BufferDesc.RefreshRate.Numerator = refleshRate;//���j�^�̃��t���b�V�����[�g�B(�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���^�C�~���O�ƂȂ�B)
		sd.BufferDesc.RefreshRate.Denominator = 1;			//�Q�ɂ�����30fps�ɂȂ�B1�ł����B
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//�T�[�t�F�X�܂��̓��\�[�X���o�̓����_�[ �^�[�Q�b�g�Ƃ��Ďg�p���܂��B
		sd.OutputWindow = hWnd;								//�o�͐�̃E�B���h�E�n���h���B
		sd.SampleDesc.Count = 1;							//1�ł����B
		sd.SampleDesc.Quality = 0;							//MSAA�Ȃ��B0�ł����B
		sd.Windowed = initParam.isWindowMode ? TRUE : FALSE;//�E�B���h�E���[�h�BTRUE�ł悢�B

		//���p����DirectX�̋@�\�Z�b�g�B
		//���̔z���D3D11CreateDeviceAndSwapChain�̈����Ƃ��Ďg���B
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
		};
		D3D_FEATURE_LEVEL confirmedFeatureLevel;

		//D3D�f�o�C�X�ƃX���b�v�`�F�C�����쐬����B
		D3D11CreateDeviceAndSwapChain(
			NULL,											//NULL�ł����B
			D3D_DRIVER_TYPE_HARDWARE,						//D3D�f�o�C�X���A�N�Z�X����h���C�o�[�̎�ށB
															//��{�I��D3D_DRIVER_TYPE_HARDWARE���w�肷��΂悢�B
			NULL,											//NULL�ł����B
			0,												//�O�ł����B
			featureLevels,									//D3D�f�o�C�X�̃^�[�Q�b�g�ƂȂ�@�\�Z�b�g���w�肷��B
			sizeof(featureLevels) / sizeof(featureLevels[0]),	//�@�\�Z�b�g�̐��B
			D3D11_SDK_VERSION,								//�g�p����DirectX�̃o�[�W�����B
															//D3D11_SDK_VERSION���w�肷��΂悢�B
			&sd,											//�X���b�v�`�F�C�����쐬���邽�߂̏��B
			&m_pSwapChain,									//�쐬�����X���b�v�`�F�C���̃A�h���X�̊i�[��B
			&m_pd3dDevice,									//�쐬����D3D�f�o�C�X�̃A�h���X�̊i�[��B
			&confirmedFeatureLevel,							//�g�p�����@�\�Z�b�g�̊i�[��B
			&m_pd3dDeviceContext							//�쐬����D3D�f�o�C�X�R���e�L�X�g�̃A�h���X�̊i�[��B
		);

		//�������ݐ�ɂȂ郌���_�����O�^�[�Q�b�g���쐬�B
		ID3D11Texture2D* pBackBuffer = NULL;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_backBuffer);
		pBackBuffer->Release();
		//�[�x�X�e���V���r���[�̍쐬�B
		{
			//�[�x�e�N�X�`���̍쐬�B
			D3D11_TEXTURE2D_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(texDesc));
			texDesc.Width = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_W());
			texDesc.Height = static_cast<UINT>(GetGraphicsEngine().GetFrameBuffer_H());
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_D32_FLOAT;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;
			m_pd3dDevice->CreateTexture2D(&texDesc, NULL, &m_depthStencil);
			//�[�x�X�e���V���r���[���쐬�B
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			ZeroMemory(&descDSV, sizeof(descDSV));
			descDSV.Format = texDesc.Format;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;
			m_pd3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
		}

		//���X�^���C�U���������B
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		m_pd3dDevice->CreateRasterizerState(&desc, &m_rasterizerState);
		m_pd3dDeviceContext->RSSetState(m_rasterizerState);

		//�f�v�X�X�e���V���X�e�[�g
		{
			D3D11_DEPTH_STENCIL_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			desc.StencilEnable = false;
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			GetD3DDevice()->CreateDepthStencilState(&desc, &m_depthStencilState);
			GetD3DDeviceContext()->OMSetDepthStencilState(m_depthStencilState, 0);
		}

		return true;
	}

	void DX11Test::Release() {
		RELEASE(m_depthStencilState);
		RELEASE(m_rasterizerState);
		RELEASE(m_depthStencil);
		RELEASE(m_depthStencilView);
		RELEASE(m_backBuffer);
		RELEASE(m_pSwapChain);
		RELEASE(m_pd3dDeviceContext);
		RELEASE(m_pd3dDevice);
	}

	void DX11Test::ChangeFrameBufferSize() {
		//���X���b�v�`�F�C�����Q�Ƃ��Ă���I�u�W�F�N�g���폜���ĂȂ���m_pSwapChain->ResizeBuffers���ł��Ȃ�
		//�o�b�N�o�b�t�@���S!!!��!!
		if (m_backBuffer) { m_backBuffer->Release(); m_backBuffer = NULL; }

		//�X���b�v�`�F�C���T�C�Y�ύX
		DXGI_SWAP_CHAIN_DESC sd;
		m_pSwapChain->GetDesc(&sd);
		HRESULT hr = m_pSwapChain->ResizeBuffers(0, (UINT)GetGraphicsEngine().GetFrameBuffer_W(), (UINT)GetGraphicsEngine().GetFrameBuffer_H(), sd.BufferDesc.Format, sd.Flags);

		//�o�b�N�o�b�t�@�č쐬
		ID3D11Texture2D* pBackBuffer = NULL;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_backBuffer);
		pBackBuffer->Release();

		//�[�x�X�e���V���̐ݒ�R�s�[
		D3D11_TEXTURE2D_DESC texDesc;
		m_depthStencil->GetDesc(&texDesc);
		texDesc.Width = (UINT)GetGraphicsEngine().GetFrameBuffer_W();
		texDesc.Height = (UINT)GetGraphicsEngine().GetFrameBuffer_H();
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		m_depthStencilView->GetDesc(&descDSV);
		descDSV.Format = texDesc.Format;

		//�[�x�X�e���V�����S!!!��!!
		if (m_depthStencil) { m_depthStencil->Release(); m_depthStencil = NULL; }
		if (m_depthStencilView) { m_depthStencilView->Release(); m_depthStencilView = NULL; }

		//�[�x�e�N�X�`���č쐬
		m_pd3dDevice->CreateTexture2D(&texDesc, NULL, &m_depthStencil);
		//�[�x�X�e���V���r���[�č쐬
		m_pd3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
	}

	void DX11Test::SetBackBufferToRenderTarget()
	{
		//�`�����ݐ���o�b�N�o�b�t�@�ɂ���B
		m_pd3dDeviceContext->OMSetRenderTargets(1, &m_backBuffer, m_depthStencilView);
	}
	void DX11Test::SwapBackBuffer()
	{
		//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�����ւ���B
		m_pSwapChain->Present(GetGraphicsEngine().GetUseVSync() ? 1 : 0, 0);
	}

}