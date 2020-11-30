#include "DWstdafx.h"
#include "CImposter.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {
	ImposterTexBank* ImposterTexBank::instance = nullptr;

//�C���|�X�^�[�̃C���X�^���V���O�`��ɂ�����g�嗦�������N���X
	void InstancingImposterParamManager::Reset(int instancingMaxNum) {
		m_instanceMax = instancingMaxNum;

		//�X�g���N�`���[�o�b�t�@������
		m_paramsSB.Init(instancingMaxNum);

		//�C���f�b�N�X�z��̊m��
		m_paramsCache = std::make_unique<CVector2[]>(instancingMaxNum);
	}
	InstancingImposterParamManager::InstancingImposterParamManager(int instancingMaxNum, ImposterTexRender* tex) : m_texture(tex) {
		Reset(instancingMaxNum);
	}
	void InstancingImposterParamManager::PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask) {
		//�J�����O����ĂȂ����̂̂݃R�s�[
		int drawNum = 0;
		for (int i = 0; i < instanceNum; i++) {
			if (drawInstanceMask[i]) {
				m_paramsSB.GetData()[drawNum] = m_paramsCache[i];
				drawNum++;
			}
		}
		//StructuredBuffer���X�V
		m_paramsSB.UpdateSubresource();
		//�V�F�[�_�[���\�[�X�ɃZ�b�g
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(
			enSkinModelSRVReg_InstancingImposterScale, 1, m_paramsSB.GetAddressOfSRV()
		);
	}
	void InstancingImposterParamManager::AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param) {
		m_paramsCache[instanceIndex].x = scale.x / (m_texture->GetModelSize()*2.0f);
		AddRotY(instanceIndex, *(float*)param);
	}
	void InstancingImposterParamManager::AddRotY(int instanceIndex, float rad) {
		m_paramsCache[instanceIndex].y = rad;
	}
	void InstancingImposterParamManager::SetInstanceMax(int instanceMax) {
		if (instanceMax > m_instanceMax) {
			Reset(instanceMax);
		}
	}
	
//�C���|�X�^�[�e�N�X�`��
	void ImposterTexRender::Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum, const CQuaternion& rotOffset, bool isJustFit) {
		//�𑜓x�E�������ݒ�
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
		//TexData������
		m_albedoTextureData.isDDS = true;//?
		m_albedoTextureData.width = texDesc.Width;
		m_albedoTextureData.height = texDesc.Height;
		m_GBufferTex[enGBufferAlbedo]->AddRef();
		m_albedoTextureData.texture.Attach(m_GBufferTex[enGBufferAlbedo].Get());
		m_GBufferSRV[enGBufferAlbedo]->AddRef();
		m_albedoTextureData.textureView.Attach(m_GBufferSRV[enGBufferAlbedo].Get());

		//���C�g�p�����[�^�[
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferLightParam].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferView[enGBufferLightParam].ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferSRV[enGBufferLightParam].ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[

		//�@��
		texDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferNormal].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferView[enGBufferNormal].ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferSRV[enGBufferNormal].ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[

		//�g�����X���[�Z���g
		texDesc.Format = DXGI_FORMAT_R8_UNORM;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferTranslucent].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferTranslucent].Get(), nullptr, m_GBufferView[enGBufferTranslucent].ReleaseAndGetAddressOf());//�����_�[�^�[�Q�b�g
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferTranslucent].Get(), nullptr, m_GBufferSRV[enGBufferTranslucent].ReleaseAndGetAddressOf());//�V�F�[�_�[���\�[�X�r���[
		
		//���[���h�s��̏�����
		CMatrix beforeWorldMatrix = model.GetWorldMatrix();//�����L�^���Ă���
		model.UpdateWorldMatrix(0.0f, CQuaternion::Identity(), 1.0f);//���[���h�s��̏�����

		//�o�C�A�X�s��擾
		CMatrix mBias, mBiasScr;
		CoordinateSystemBias::GetBias(mBias, mBiasScr, model.GetFBXUpAxis(), model.GetFBXCoordinateSystem());
		mBias.Mul(mBiasScr, mBias);

		//�o�E���f�B���O�{�b�N�X���烂�f���̃T�C�Y�����߂�
		m_imposterMaxSize = 0.0f;
		m_imposterSizeZ = 0.0f;
		m_boundingBoxMaxSize = { 0.0f };
		m_boundingBoxMinSize = { 0.0f };		
		CVector3 CenterV, ExpendV;//isJustFit�p
		{
			model.GetBoundingBox(m_boundingBoxMinSize, m_boundingBoxMaxSize);
			mBias.Mul3x3(m_boundingBoxMinSize);//�o�C�A�X�̓K��
			mBias.Mul3x3(m_boundingBoxMaxSize);//�o�C�A�X�̓K��

			//��ӂ̍ő咷���߂�
			CQuaternion rotY, rotX, rotM;
			for (int i = 0; i < (int)(m_partNumX * m_partNumY); i++) {
				//���[�܂ōs����
				if (i % m_partNumX == 0) {
					//����]���Z�b�g
					rotY = CQuaternion::Identity();
					rotY.SetRotation(CVector3::AxisY(), CMath::PI * -1.0f);

					//�c��]�i�߂�
					if (i != 0 && m_partNumY > 1) {
						float angle = CMath::PI / (m_partNumY - 1) * (i / m_partNumX);
						rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f + angle);
					}
					else {
						rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f);
					}
				}

				//��]�s��쐬
				rotM.Concatenate(rotY, rotX);
				rotM.Concatenate(rotM, rotOffset);//��]�I�t�Z�b�g�K�p
				model.UpdateWorldMatrix(0.0f, rotM, 1.0f);

				//�ӂ̒������߂�
				CVector3 minV, maxV;
				model.GetUpdatedBoundingBox(minV, maxV);
				m_imposterMaxSize = max(m_imposterMaxSize, abs(minV.x));
				m_imposterMaxSize = max(m_imposterMaxSize, abs(minV.y));
				m_imposterMaxSize = max(m_imposterMaxSize, abs(maxV.x));
				m_imposterMaxSize = max(m_imposterMaxSize, abs(maxV.y));
				m_imposterSizeZ = max(m_imposterSizeZ, abs(minV.z - maxV.z));

				//isJustFit�p
				CenterV = (minV + maxV) * 0.5f;
				ExpendV = maxV - CenterV;

				//����]�i�߂�
				rotY.Concatenate(CQuaternion(CVector3::AxisY(), CMath::PI2 / (m_partNumX - 1)));
			}
		}
	
		//���f���̃J���������̑傫��
		m_toCamDirSize.Init(m_partNumX*m_partNumY);

		//�C���|�X�^�e�N�X�`���̍쐬
		Render(model, rotOffset, isJustFit, CenterV, ExpendV);

		//���f���̃��[���h�s���߂�
		model.SetWorldMatrix(beforeWorldMatrix, true);

		//StructuredBuffer���X�V
		m_toCamDirSize.UpdateSubresource();
	}

	void ImposterTexRender::VSSetSizeToCameraSRV() {
		//�V�F�[�_�[���\�[�X�ɃZ�b�g
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(
			enSkinModelSRVReg_ImposterSizeToCamera, 1, m_toCamDirSize.GetAddressOfSRV()
		);
	}

	void ImposterTexRender::Render(SkinModel& model, const CQuaternion& rotOffset, bool isJustFit, const CVector3& justFitCenter, const CVector3& justFitExpand) {
		//GPU�C�x���g�̊J�n
		GetGraphicsEngine().BeginGPUEvent(L"RenderImposter");

		ID3D11DeviceContext* DC = GetGraphicsEngine().GetD3DDeviceContext();

		//�J�����̃Z�b�g�A�b�v		
		GameObj::NoRegisterOrthoCamera imposterCam;
		imposterCam.SetUp(CVector3::Up());
		if (isJustFit) {
			float size = max(justFitExpand.x, justFitExpand.y)*2.0f;
			imposterCam.SetWidth(size);
			imposterCam.SetHeight(size);
			imposterCam.SetPos(justFitCenter + CVector3::AxisZ()*(justFitExpand.z +100.0f));
			imposterCam.SetTarget(justFitCenter);
			imposterCam.SetFar(justFitExpand.z*2.0f + 100.0f);
		}
		else {
			imposterCam.SetWidth(m_imposterMaxSize * 2.0f);
			imposterCam.SetHeight(m_imposterMaxSize * 2.0f);
			imposterCam.SetPos({ 0.0f,0.0f,m_imposterSizeZ*0.5f + 100.0f });
			imposterCam.SetTarget(CVector3::Zero());
			imposterCam.SetFar(m_imposterSizeZ + 100.0f);
		}
		//�����̃J�����͎蓮�ōX�V����...
		imposterCam.UpdateMatrix();

		//�J�����ۑ�
		GameObj::ICamera* beforeCam = GetMainCamera();
		//�J�����ύX
		SetMainCamera(&imposterCam);

		//�r���[�|�[�g�L�^
		D3D11_VIEWPORT beforeViewport; UINT kaz = 1;
		DC->RSGetViewports(&kaz, &beforeViewport);


		//�f�v�X
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilTex; //�f�v�X�X�e���V���e�N�X�`��
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;//�f�v�X�X�e���V���r���[
		//�f�v�X�e�N�X�`��
		D3D11_TEXTURE2D_DESC texDesc;
		m_GBufferTex[0]->GetDesc(&texDesc);
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		GetGraphicsEngine().GetD3DDevice()->CreateTexture2D(&texDesc, NULL, depthStencilTex.ReleaseAndGetAddressOf());		
		//�f�v�X�X�e���V��
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ZeroMemory(&dsv_desc, sizeof(dsv_desc));
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		GetGraphicsEngine().GetD3DDevice()->CreateDepthStencilView(depthStencilTex.Get(), &dsv_desc, depthStencilView.ReleaseAndGetAddressOf());
		
		//G�o�b�t�@�o�̓s�N�Z���V�F�[�_
		SkinModelEffectShader imposterPS;
		imposterPS.Load("Preset/shader/Imposter.fx", "PSMain_RenderImposter", Shader::EnType::PS);

		//G�o�b�t�@���N���A
		float clearColor[enGBufferNum][4] = {
			{ 0.0f, 0.0f, 0.0f, 0.0f }, //enGBufferAlbedo
			{ 0.5f, 0.5f, 1.0f, 1.0f }, //enGBufferNormal
			{ 0.0f, 0.0f, 0.0f, 1.0f }, //enGBufferLightParam
			{ 1.0f, 1.0f, 1.0f, 1.0f }, //enGBufferTranslucent
		};
		for (int i = 0; i < enGBufferNum; i++) {
			DC->ClearRenderTargetView(m_GBufferView[i].Get(), clearColor[i]);
		}
		//�f�v�X�X�e���V�����N���A
		DC->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// RenderTarget�ݒ�
		ID3D11RenderTargetView* renderTargetViews[enGBufferNum] = { nullptr };
		for (unsigned int i = 0; i < enGBufferNum; i++) {
			renderTargetViews[i] = m_GBufferView[i].Get();
		}
		DC->OMSetRenderTargets(enGBufferNum, renderTargetViews, depthStencilView.Get());

		//�V�F�[�_�[
		std::list<SKEShaderPtr> beforeShaders;
		model.FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//�V�F�[�_�ۑ�
				beforeShaders.emplace_back(mat->GetPS());
				//�V�F�[�_�ύX
				mat->SetPS(&imposterPS);
			}
		);

		//�r���[�|�[�g
		D3D11_VIEWPORT viewport;
		viewport.Width = (float)m_gbufferSizeX / m_partNumX;
		viewport.Height = (float)m_gbufferSizeY / m_partNumY;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//���f���`��
		int indY = 0;
		CQuaternion rotY, rotX, rotM;
		for (int i = 0; i < (int)(m_partNumX*m_partNumY); i++) {
			//���[�܂ōs����
			if (i%m_partNumX == 0) {
				//�r���[�|�[�g�c�ɂ��炷
				viewport.TopLeftX = 0;
				if (i != 0) { viewport.TopLeftY += viewport.Height; }

				//����]���Z�b�g
				rotY = CQuaternion::Identity();
				rotY.SetRotation(CVector3::AxisY(), CMath::PI*-1.0f);

				//�c��]�i�߂�
				if (i != 0 && m_partNumY > 1) {
					float angle = CMath::PI / (m_partNumY - 1) * (i / m_partNumX);
					rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f + angle);
				}
				else {
					rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f);
				}

				//�C���f�b�N�X�i�߂�
				if (i != 0) { indY++; }
			}

			//���f���̉�]	
			rotM.Concatenate(rotY, rotX);
			rotM.Concatenate(rotM, rotOffset);//��]�I�t�Z�b�g�K�p
			model.UpdateWorldMatrix(0.0f, rotM, 1.0f);
				
			//�r���[�|�[�g�ݒ�
			DC->RSSetViewports(1, &viewport);
			//���f���`��
			model.Draw();

			//�o�E���f�B���O�{�b�N�X�̕\��
			/*{
				CVector3 min, max;
				model.GetUpdatedBoundingBox(min, max);
				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

				ImmediateDrawLine({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
			}
			{
				CVector3 min = justFitCenter - justFitExpand, max = justFitCenter + justFitExpand; 
				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });

				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });

				ImmediateDrawLine({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
			}*/
			//float size = max(justFitExpand.x, justFitExpand.y) * 2.0f;
			//ImmediateDrawQuad({ -size ,-size,size }, { size ,size,size }, { 1.0f,1.0f,0.0f,1.0f });

			//���f���̃J���������̑傫�����L�^
			float toCamDirMaxSize = 0.0f;
			CVector3 toCamDir;
			
			toCamDir = m_boundingBoxMinSize;
			rotM.Multiply(toCamDir);
			toCamDirMaxSize = toCamDir.z;

			toCamDir = m_boundingBoxMaxSize;
			rotM.Multiply(toCamDir);
			toCamDirMaxSize = max(toCamDirMaxSize, toCamDir.z);

			m_toCamDirSize.GetData()[indY*m_partNumX + i%m_partNumX] = toCamDirMaxSize;
			
			//�r���[�|�[�g���ɂ��炷
			viewport.TopLeftX += viewport.Width;

			//����]�i�߂�
			rotY.Concatenate(CQuaternion(CVector3::AxisY(), CMath::PI2 / (m_partNumX - 1)));
		}

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
		GetGraphicsEngine().ResetRasterizerState();

		//�����_�[�^�[�Q�b�g����
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//�r���[�|�[�g�߂�
		DC->RSSetViewports(1, &beforeViewport);

		//�J�����߂�
		SetMainCamera(beforeCam);

		//GPU�C�x���g�̏I��
		GetGraphicsEngine().EndGPUEvent();
	}

	float ImposterTexRender::GetDirectionOfCameraSize(int x, int y)const {
		return m_toCamDirSize.GetData()[(m_partNumY - 1 + y)*m_partNumX + x];
	}

//�C���|�X�^�[�e�N�X�`���o���N
	ImposterTexRender* ImposterTexBank::Load(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, const CQuaternion& rotOffset, bool isJustFit) {
		//�n�b�V���쐬
		int index = Util::MakeHash(identifier);

		//�n�b�V����map��key�Ƃ��Ďg�p
		if (m_impTexMap.count(index) > 0) {
			//��������
			return m_impTexMap[index];
		}
		else {
			//����
			ImposterTexRender* ptr = new ImposterTexRender;
			ptr->Init(model, resolution, partNum, rotOffset, isJustFit);
			m_impTexMap.emplace(index, ptr);
			return ptr;
		}
	}

	ImposterTexRender* ImposterTexBank::Get(const wchar_t* identifier) {
		//�n�b�V���쐬
		int index = Util::MakeHash(identifier);

		//�n�b�V����map��key�Ƃ��Ďg�p
		if (m_impTexMap.count(index) > 0) {
			//����
			return m_impTexMap[index];
		}
		else {
			//�Ȃ�
			return nullptr;
		}
	}

	void ImposterTexBank::Release() {
		for (auto& T : m_impTexMap) {
			delete T.second;
		}
		m_impTexMap.clear();
	}
	
//�C���|�X�^�[
	bool CImposter::Init(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, int instancingNum) {
		//�e�N�X�`���ǂݍ���
		m_texture = nullptr;
		m_texture = ImposterTexBank::GetInstance().Load(identifier, model, resolution, partNum);
		if (!m_texture) { return false; }
		//������
		InnerInit(identifier, instancingNum);
		return true;
	}
	bool CImposter::Init(const wchar_t* identifier, int instancingNum) {
		//�e�N�X�`���ǂݍ���
		m_texture = nullptr;
		m_texture = ImposterTexBank::GetInstance().Get(identifier);
		if (!m_texture) { return false; }
		//������
		InnerInit(identifier, instancingNum);
		return true;
	}
	void CImposter::InnerInit(const wchar_t* identifier, int instancingNum){
		std::wstring moji = L"CImposter-";
		moji += identifier; moji += L"-CImposter";

		//�r���{�[�h
		m_billboard.Init(m_texture->GetSRV(ImposterTexRender::enGBufferAlbedo), instancingNum, moji.c_str());

		//�V�F�[�_�ǂݍ���
		if (!m_s_isShaderLoaded) {
			{
				//�C���X�^���V���O�p�V�F�[�_
				D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enInstancing].Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);
				m_s_vsZShader[enInstancing].Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);

				D3D_SHADER_MACRO macros[] = {
					"INSTANCING", "1",
					NULL, NULL
				};
				m_s_imposterPS[enInstancing].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS, "INSTANCING", macros);

				D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1","INSTANCING", "1", NULL, NULL };
				m_s_zShader[enInstancing].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE_INSTANCING", macrosZ);
			}
			{
				D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enNormal].Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);
				m_s_vsZShader[enNormal].Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);

				m_s_imposterPS[enNormal].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);

				D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
				m_s_zShader[enNormal].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
			}
			m_s_isShaderLoaded = true;
		}

		//���낢��ݒ�
		m_billboard.GetModel().GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//�e�N�X�`��
				mat->SetNormalTexture(m_texture->GetSRV(ImposterTexRender::enGBufferNormal));
				mat->SetLightingTexture(m_texture->GetSRV(ImposterTexRender::enGBufferLightParam));
				mat->SetTranslucentTexture(m_texture->GetSRV(ImposterTexRender::enGBufferTranslucent));
				//�V�F�[�_
				mat->SetVS(&m_s_vsShader[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
				mat->SetVSZ(&m_s_vsZShader[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
				mat->SetPS(&m_s_imposterPS[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
				mat->SetPSZ(&m_s_zShader[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
			}
		);

		//�������ݒ�
		m_billboard.GetModel().GetSkinModel().SetImposterPartNum(m_texture->GetPartNumX(), m_texture->GetPartNumY());
		
		//�`��O�����̐ݒ�
		if (m_billboard.GetIsInstancing()) {
			m_billboard.GetInstancingModel().GetInstancingModel()->SetPreDrawFunction([this]() { m_texture->VSSetSizeToCameraSRV(); });
		}
		else {
			m_billboard.GetModel().GetSkinModel().SetPreDrawFunction(L"DW_SetImposterSizeToCamera",[this](SkinModel*) { m_texture->VSSetSizeToCameraSRV(); });
		}

		//�C���X�^���V���O
		if (m_billboard.GetIsInstancing()) {
			//IInstanceData��ݒ�
			if (!m_billboard.GetInstancingModel().GetInstancingModel()->GetIInstanceData(L"InstancingImposterParamManager")) {
				//�V�K�쐬
				m_billboard.GetInstancingModel().GetInstancingModel()->AddIInstanceData(L"InstancingImposterParamManager", std::make_unique<InstancingImposterParamManager>(m_billboard.GetInstancingModel().GetInstancingModel()->GetInstanceMax(), m_texture));
			}
			//Y����]�l�̃|�C���^��ݒ�
			if (m_billboard.GetIsInstancing()) {
				m_billboard.GetInstancingModel().SetParamPtr(&m_rotYrad);
			}
		}

		//�X�P�[��������
		SetScale(1.0f);

		m_isInit = true;
	}

	/*
	void CImposter::CalcWorldMatrixAndIndex(bool isShadowDrawMode, const SkinModel& model, const ImposterTexRender& texture, const CVector3& pos, float scale, CVector3& position_return, CQuaternion& rotation_return, float& scale_return, int& index_x, int& index_y) {
		
		if (!GetMainCamera()) {
#ifndef DW_MASTER
			OutputDebugStringA("CImposter::CalcWorldMatrixAndIndex() �J�������ݒ肳��Ă��܂���B\n");
#endif
			return;
		}

		//�C���|�X�^�[�p�C���f�b�N�X�v�Z
		CVector3 polyDir = GetMainCamera()->GetPos() - pos; polyDir.Normalize();
		
		//X����]
		CVector3 axisDir = polyDir; axisDir.x = CVector2(polyDir.x, polyDir.z).Length();
		float XRot = std::atan2(axisDir.y, axisDir.x);
		index_y = (int)std::round(XRot / CMath::PI * texture.GetPartNumY()) - (int)(texture.GetPartNumY() / 2.0f - 0.5f);

		//Y����]		
		float YRot = std::atan2(polyDir.x, polyDir.z);
		index_x = (int)std::round(-YRot / CMath::PI2 * texture.GetPartNumX()) + (int)(texture.GetPartNumX() / 2.0f - 0.5f);

		//�J���������Ƀ��f���T�C�Y�����W���炷
		//�����܂�h�~
		if (!isShadowDrawMode) {
			polyDir *= scale * texture.GetDirectionOfCameraSize(index_x, index_y);
		}

		//��]
		CQuaternion rot;
		rot.SetRotation(CVector3::AxisY(), index_x * -(CMath::PI2 / (texture.GetPartNumX() - 1)) + CMath::PI2);
		rot.Multiply(CQuaternion(CVector3::AxisX(), -index_y * -(CMath::PI / (texture.GetPartNumY() - 1)) + CMath::PI*0.5f));

		//�Ԃ�
		position_return = pos;// +polyDir;
		rotation_return = CQuaternion::Identity();// rot;
		scale_return = scale * texture.GetModelSize()*2.0f;
	}
	*/
}