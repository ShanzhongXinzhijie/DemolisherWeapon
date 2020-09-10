#include "DWstdafx.h"
#include "Model.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

	namespace {
		/// <summary>
		/// �p�X��whar_t�֕ϊ�...
		/// </summary>
		/// <param name="c"></param>
		/// <param name="wchar_nonMake"></param>
		void ConvertWchar(const char* c, std::unique_ptr<wchar_t[]>& wchar_nonMake) {
			size_t iReturnValue;
			size_t size = strlen(c) + 1;
			wchar_nonMake = std::make_unique<wchar_t[]>(size);
			errno_t err = mbstowcs_s(
				&iReturnValue,
				wchar_nonMake.get(),
				size, //��̃T�C�Y
				c,
				size - 1 //�R�s�[����ő啶����
			);
			if (err != 0) {
				DW_ERRORBOX(true, "mbstowcs_s errno:%d", err)
			}
		}
	}

	IMaterial::IMaterial(bool isSkinModel, const tkEngine::CTkmFile::SMaterial& tkmMat,int number){
		//���O�ݒ�
		std::wstring name = L"TKM_Material_" + std::to_wstring(number);
		//�e�N�X�`��������
		std::unique_ptr<wchar_t[]> path;
		ConvertWchar(tkmMat.albedoMapFileName.c_str(), path);
		m_materialData.InitAlbedoTexture(path.get());
		ConvertWchar(tkmMat.albedoMapFileName.c_str(), path);
		m_materialData.InitNormalTexture(path.get());
		ConvertWchar(tkmMat.albedoMapFileName.c_str(), path);
		m_materialData.InitLightingTexture(path.get());
		//������
		m_materialData.Init(isSkinModel, name);
	}

	void MaterialDX11::Apply() {
		auto deviceContext = GetGraphicsEngine().GetD3DDeviceContext();
		auto m_ptrUseMaterialSetting = &m_materialData.GetUsingMaterialSetting();

		//�V�F�[�_�[���[�h�ɂ��������V�F�[�_���Z�b�g
		switch (GetGraphicsEngine().GetModelDrawMode().GetShaderMode()) {
		case ModelDrawMode::enZShader://Z�l�̕`��

			//���_�V�F�[�_
			deviceContext->VSSetShader((ID3D11VertexShader*)m_ptrUseMaterialSetting->GetVSZ()->GetBody(), NULL, 0);

			//�s�N�Z���V�F�[�_
			if (m_ptrUseMaterialSetting->GetPSZ() == m_materialData.GetDefaultPSZ(false)) {
				deviceContext->PSSetShader((ID3D11PixelShader*)m_materialData.GetDefaultPSZ(m_ptrUseMaterialSetting->GetIsUseTexZShader())->GetBody(), NULL, 0);
			}
			else {
				deviceContext->PSSetShader((ID3D11PixelShader*)m_ptrUseMaterialSetting->GetPSZ()->GetBody(), NULL, 0);
			}

			break;

		default://�ʏ�`��

			//�f�t�H���g�V�F�[�_�̃}�N����؂�ւ���
			int macroind = 0;
			if (m_ptrUseMaterialSetting->GetIsMotionBlur()) { macroind |= SkinModelEffectShader::enMotionBlur; }
			if (m_ptrUseMaterialSetting->GetNormalTexture()) { macroind |= SkinModelEffectShader::enNormalMap; }
			if (m_ptrUseMaterialSetting->GetAlbedoTexture()) { macroind |= SkinModelEffectShader::enAlbedoMap; }
			if (m_ptrUseMaterialSetting->GetLightingTexture()) { macroind |= SkinModelEffectShader::enLightingMap; }
			if (m_ptrUseMaterialSetting->GetTranslucentTexture()) { macroind |= SkinModelEffectShader::enTranslucentMap; }

			//���_�V�F�[�_
			deviceContext->VSSetShader((ID3D11VertexShader*)m_ptrUseMaterialSetting->GetVS().Get(macroind)->GetBody(), NULL, 0);

			//�s�N�Z���V�F�[�_
			deviceContext->PSSetShader((ID3D11PixelShader*)m_ptrUseMaterialSetting->GetPS().Get(macroind)->GetBody(), NULL, 0);

			break;
		}

		//�e�N�X�`��
		if (m_ptrUseMaterialSetting->GetAlbedoTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, m_ptrUseMaterialSetting->GetAddressOfAlbedoTexture());
		}
		if (m_ptrUseMaterialSetting->GetNormalTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, m_ptrUseMaterialSetting->GetAddressOfNormalTexture());
		}
		if (m_ptrUseMaterialSetting->GetLightingTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_LightngTexture, 1, m_ptrUseMaterialSetting->GetAddressOfLightingTexture());
		}
		if (m_ptrUseMaterialSetting->GetTranslucentTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_TranslucentTexture, 1, m_ptrUseMaterialSetting->GetAddressOfTranslucentTexture());
		}

		//�萔�o�b�t�@
		deviceContext->UpdateSubresource(m_materialData.GetConstantBufferDX11().Get(), 0, NULL, &m_ptrUseMaterialSetting->GetMaterialParam(), 0, 0);
		deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, m_materialData.GetConstantBufferDX11().GetAddressOf());
	}

	void MaterialDX12::Apply() {
		//�����Ń}�e���A���̏���
		//�f�B�X�N���v�^�ݒ�?
	}

	void CMeshParts::InitFromTkmFile(const tkEngine::CTkmFile& tkmFile) {
		m_meshs.resize(tkmFile.GetNumMesh());
		int meshNo = 0;
		tkmFile.QueryMeshParts([&](const tkEngine::CTkmFile::SMesh& mesh) {
			CreateMeshFromTkmMesh(mesh, meshNo);
			meshNo++;
		});
	}

	void CMeshParts::CreateMeshFromTkmMesh(const tkEngine::CTkmFile::SMesh& tkmMesh, int meshNo)
	{
		int numVertex = (int)tkmMesh.vertexBuffer.size();
		int vertexStride = sizeof(tkEngine::CTkmFile::SVertex);

		//���b�V���쐬
		auto mesh = std::make_unique<SMesh>();
		mesh->m_skinFlags.reserve(tkmMesh.materials.size());

		//���_�o�b�t�@�쐬
		if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
			mesh->m_vertexBuffer = std::make_unique<VertexBufferDX11>();
		}
		if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
			mesh->m_vertexBuffer = std::make_unique<VertexBufferDX12>();
		}
		mesh->m_vertexBuffer->Init(numVertex, vertexStride, (void*)&tkmMesh.vertexBuffer[0]);

		//�X�L�������邩?
		auto SetSkinFlag = [&](int index) {
			if (tkmMesh.vertexBuffer[index].skinWeights.x > 0.0f) {
				//�X�L��������B
				mesh->m_skinFlags.push_back(1);
			}
			else {
				//�X�L���Ȃ��B
				mesh->m_skinFlags.push_back(0);
			}
		};

		//�C���f�b�N�X�o�b�t�@���쐬�B
		if (!tkmMesh.indexBuffer16Array.empty()) {
			//�C���f�b�N�X�̃T�C�Y��2byte
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer16Array.size());
			for (auto& tkIb : tkmMesh.indexBuffer16Array) {

				std::vector<unsigned long> dstib;
				for (auto& srcib : tkIb.indices) {
					dstib.emplace_back((unsigned long)srcib);
				}

				std::unique_ptr<IIndexBuffer> ib;
				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					ib = std::make_unique<IndexBufferDX11>();
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					ib = std::make_unique<IndexBufferDX12>();
				}
				ib->Init((int)tkIb.indices.size(), &dstib[0]);

				//�X�L�������邩�ǂ�����ݒ肷��B
				SetSkinFlag(tkIb.indices[0]);

				mesh->m_indexBufferArray.push_back(std::move(ib));
			}
		}
		else {
			//�C���f�b�N�X�̃T�C�Y��4byte
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer32Array.size());
			for (auto& tkIb : tkmMesh.indexBuffer32Array) {

				std::vector<unsigned long> dstib;
				for (auto& srcib : tkIb.indices) {
					dstib.emplace_back((unsigned long)srcib);
				}

				std::unique_ptr<IIndexBuffer> ib;
				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					ib = std::make_unique<IndexBufferDX11>();
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					ib = std::make_unique<IndexBufferDX12>();
				}
				ib->Init((int)tkIb.indices.size(), &dstib[0]);

				//�X�L�������邩�ǂ�����ݒ肷��B
				SetSkinFlag(tkIb.indices[0]);

				mesh->m_indexBufferArray.push_back(std::move(ib));
			}
		}
		//�}�e���A�����쐬�B
		int ind = 0;
		mesh->m_materials.reserve(tkmMesh.materials.size());
		for (auto& tkmMat : tkmMesh.materials) {
			//�쐬
			std::unique_ptr<IMaterial> mat;
			if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
				 mat = std::make_unique<MaterialDX11>(mesh->m_skinFlags[ind], tkmMat, ind);
			}
			if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
				mat = std::make_unique<MaterialDX12>(mesh->m_skinFlags[ind], tkmMat, ind);
			}
			//�ۑ�
			mesh->m_materials.push_back(std::move(mat));

			ind++;
		}

		m_meshs[meshNo] = std::move(mesh);
	}

	void CMeshParts::Draw() {
		for (auto& mesh : m_meshs) {
			//���_�o�b�t�@��ݒ�
			mesh->m_vertexBuffer->Attach();
			//�}�e���A�����ƂɃh���[�B
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//�C���f�b�N�X�o�b�t�@��ݒ�
				mesh->m_indexBufferArray[matNo]->Attach();
				//�}�e���A���̐ݒ�
				mesh->m_materials[matNo]->Apply();

				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					//�g�|���W�[��ݒ�
					GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					//�`��
					GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->DrawIndexed(mesh->m_indexBufferArray[matNo]->GetIndexNum(), 0, 0);
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					//�g�|���W�[��ݒ�
					GetGraphicsEngine().GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					//�`��
					GetGraphicsEngine().GetCommandList()->DrawIndexedInstanced(mesh->m_indexBufferArray[matNo]->GetIndexNum(), 1, 0, 0, 0);
				}
			}
		}

		/// <summary>
		/// //
		/// </summary>
		/// 
		/*if (m_isCreateDescriptorHeap == false) {
			//�f�B�X�N���v�^�q�[�v���쐬�B
			CreateDescriptorHeaps();
		}

		auto& ge12 = g_graphicsEngine->As<CGraphicsEngineDx12>();
		//�����_�����O�R���e�L�X�g��Dx12�łɃ_�E���L���X�g
		auto& rc12 = rc.As<CRenderContextDx12>();
		auto& lightMgr = ge12.GetLightManager()->As<CLightManagerDx12>();

		//���b�V�����ƂɃh���[
		//�v���~�e�B�u�̃g�|���W�[�̓g���C�A���O�����X�g�̂݁B
		rc12.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�萔�o�b�t�@���X�V����B
		SConstantBuffer cb;
		cb.mWorld = mWorld;
		cb.mView = mView;
		cb.mProj = mProj;
		m_commonConstantBuffer.Update(&cb);

		if (m_boneMatricesStructureBuffer.IsInited()) {
			//�{�[���s����X�V����B
			m_boneMatricesStructureBuffer.Update(m_skeleton->GetBoneMatricesTopAddress());
		}

		int descriptorHeapNo = 0;
		for (auto& mesh : m_meshs) {
			//���_�o�b�t�@��ݒ�B
			rc12.SetVertexBuffer(mesh->m_vertexBuffer);
			//�}�e���A�����ƂɃh���[�B
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//���̃}�e���A�����\���Ă��郁�b�V���̕`��J�n�B
				mesh->m_materials[matNo]->BeginRender(rc, mesh->skinFlags[matNo]);

				auto& descriptorHeap = m_descriptorHeap.at(descriptorHeapNo);

				rc12.SetDescriptorHeap(descriptorHeap);
				descriptorHeapNo++;
				//�C���f�b�N�X�o�b�t�@��ݒ�B
				auto& ib = mesh->m_indexBufferArray[matNo];
				rc12.SetIndexBuffer(ib);

				//�h���[�B
				rc12.DrawIndexed(ib->GetCount());
			}
		}*/
	}

	void CModel::LoadTkmFileAsync(const char* filePath)
	{
		m_tkmFile.LoadAsync(filePath);
	}
	void CModel::LoadTkmFile(const char* filePath)
	{
		m_tkmFile.Load(filePath);
	}
	bool CModel::IsInited() const
	{
		return m_tkmFile.IsLoaded();
	}
	void CModel::CreateMeshParts()
	{
		if (m_tkmFile.IsLoaded() == false) {
			DW_WARNING_BOX(true, "���̊֐���tkm�t�@�C���̃��[�h���������Ă���Ăяo���Ă��������B");
			return;
		}
		m_meshParts.InitFromTkmFile(m_tkmFile);
	}
	void CModel::Draw()
	{
		m_meshParts.Draw();
	}

}