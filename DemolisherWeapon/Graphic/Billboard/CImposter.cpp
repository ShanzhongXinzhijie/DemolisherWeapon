#include "DWstdafx.h"
#include "CImposter.h"

namespace DemolisherWeapon {

	void CImposter::Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum) {
		SkinModel model;
		model.Init(filepath);
		model.UpdateWorldMatrix(0.0f,CQuaternion::Identity(),1.0f);
		Init(model, resolution, partNum);
	}

	void CImposter::Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum) {

		m_gbufferSizeX = (UINT)resolution.x; m_gbufferSizeY = (UINT)resolution.y;
		m_partNumX = (UINT)partNum.x; m_partNumY = (UINT)partNum.y;

		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//�e�N�X�`���쐬
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = (UINT)resolution.x;
		texDesc.Height = (UINT)resolution.y;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		//�A���x�h
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferAlbedo].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferAlbedo].Get(), nullptr, m_GBufferView[enGBufferAlbedo].ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferAlbedo].Get(), nullptr, m_GBufferSRV[enGBufferAlbedo].ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[

		//���C�g�p�����[�^�[
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferLightParam].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferView[enGBufferLightParam].ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferSRV[enGBufferLightParam].ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[

		//�@��
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferNormal].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferView[enGBufferNormal].ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferSRV[enGBufferNormal].ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[

		//�f�v�X
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_depthStencilTex.ReleaseAndGetAddressOf());
		//�f�v�X�X�e���V��
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ZeroMemory(&dsv_desc, sizeof(dsv_desc));
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		ge.GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex.Get(), &dsv_desc, m_depthStencilView.ReleaseAndGetAddressOf());
	
		//G�o�b�t�@�o�̓s�N�Z���V�F�[�_
		m_imposterPS.Load("Preset/shader/Imposter.fx", "PSMain_RenderImposter", Shader::EnType::PS);

		//�r���{�[�h
		m_billboard.Init(m_GBufferSRV[enGBufferAlbedo].Get());
		m_billboardPS.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);
		m_billboard.GetModel().GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetNormalTexture(m_GBufferSRV[enGBufferNormal].Get());
				mat->SetLightingTexture(m_GBufferSRV[enGBufferLightParam].Get());
				mat->SetPS(&m_billboardPS);
			}
		);

		//�o�E���f�B���O�{�b�N�X���烂�f���̃T�C�Y�����߂�
		m_imposterMaxSize = 0.0f;
		model.FindMeshes(
			[&](const std::shared_ptr<DirectX::ModelMesh>& meshes) {
				CVector3 size, extents;
				extents = meshes->boundingBox.Extents;

				size = meshes->boundingBox.Center;
				size += extents;
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y,max(size.x, size.z)));

				size = meshes->boundingBox.Center;
				size -= extents;
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y,max(size.x, size.z)));
			}
		);

		//�J�����̃Z�b�g�A�b�v		
		GameObj::NoRegisterOrthoCamera imposterCam;
		imposterCam.SetWidth(m_imposterMaxSize*2.0f);
		imposterCam.SetHeight(m_imposterMaxSize*2.0f);
		imposterCam.SetPos({ 0.0f,0.0f,CVector3(m_imposterMaxSize).Length() + 100.0f });
		imposterCam.SetTarget(CVector3::Zero());
		imposterCam.SetUp(CVector3::Up());
		//�����̃J�����͎蓮�ōX�V����...
		imposterCam.UpdateMatrix();

		//�J�����ۑ�
		GameObj::ICamera* beforeCam = GetMainCamera();
		//�J�����ύX
		SetMainCamera(&imposterCam);

		//�C���|�X�^�̍쐬
		RenderImposter(model);

		//�J�����߂�
		SetMainCamera(beforeCam);

		//�X�P�[��������
		SetScale(1.0f);
	}

	void CImposter::RenderImposter(SkinModel& model) {
		//GPU�C�x���g�̊J�n
		GetGraphicsEngine().BeginGPUEvent(L"RenderImposter");

		//G�o�b�t�@���N���A
		float clearColor[enGBufferNum][4] = {
			{ 0.5f, 0.5f, 0.5f, 0.0f }, //enGBufferAlbedo
			{ 0.0f, 1.0f, 0.0f, 1.0f }, //enGBufferNormal
			{ 0.0f, 0.0f, 0.0f, 1.0f }, //enGBufferLightParam
		};
		for (int i = 0; i < enGBufferNum; i++) {
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_GBufferView[i].Get(), clearColor[i]);
		}
		//�f�v�X�X�e���V�����N���A
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		//D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL

		// RenderTarget�ݒ�
		ID3D11RenderTargetView* renderTargetViews[enGBufferNum] = { nullptr };
		for (unsigned int i = 0; i < enGBufferNum; i++) {
			renderTargetViews[i] = m_GBufferView[i].Get();
		}
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(enGBufferNum, renderTargetViews, m_depthStencilView.Get());

		//�V�F�[�_�[
		std::list<SKEShaderPtr> beforeShaders;
		model.FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//�V�F�[�_�ۑ�
				beforeShaders.emplace_back(mat->GetPS());
				//�V�F�[�_�ύX
				mat->SetPS(&m_imposterPS);
			}
		);
		
		//�r���[�|�[�g
		D3D11_VIEWPORT viewport;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//�r���[�|�[�g�L�^
		D3D11_VIEWPORT beforeViewport; UINT kaz = 1;
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &beforeViewport);

		//���f���`��
		{
			//���f���̉�]	

			//�r���[�|�[�g�ݒ�
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = (float)m_gbufferSizeX;
			viewport.Height = (float)m_gbufferSizeY;
			//�r���[�|�[�g�ݒ�
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &viewport);
			
			//�C���X�^���V���O

			//���f���`��
			model.Draw();
		}

		//�r���[�|�[�g�߂�
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &beforeViewport);

		//�V�F�[�_�[�߂�
		model.FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//�V�F�[�_�ύX
				mat->SetPS(beforeShaders.front());
				//�擪�폜
				beforeShaders.pop_front();
			}
		);

		//���X�^���C�U�[�X�e�[�g�߂�
		GetEngine().GetGraphicsEngine().ResetRasterizerState();

		//�����_�[�^�[�Q�b�g����
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//GPU�C�x���g�̏I��
		GetGraphicsEngine().EndGPUEvent();
	}

}