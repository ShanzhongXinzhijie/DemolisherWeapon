#include "DWstdafx.h"
#include "MeshData.h"

namespace DemolisherWeapon {
	namespace Physics2 {

		void MeshData::CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix) {
#ifndef DW_DX12_TEMPORARY
			//�o�C�A�X�s��̍쐬
			CMatrix mBias, mBiasScr;
			CoordinateSystemBias::GetBias(mBias, mBiasScr, model.GetFBXUpAxis(), model.GetFBXCoordinateSystem());
			mBias.Mul(mBiasScr, mBias);
			//�I�t�Z�b�g�s��ƍ�����
			if (offsetMatrix != nullptr) {
				mBias.Mul(mBias, (*offsetMatrix));
			}

			//���b�V���̓ǂݏo��
			model.FindMesh([&](const auto& mesh) {
				ID3D11DeviceContext* deviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();
				//���_�o�b�t�@���쐬�B
				{
					//���_�o�b�t�@�̃|�C���^����ɓ����
					D3D11_MAPPED_SUBRESOURCE subresource;
					HRESULT hr = deviceContext->Map(mesh->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subresource);
					if (FAILED(hr)) {
						return;
					}

					//���_�������߂�
					D3D11_BUFFER_DESC bufferDesc;
					mesh->vertexBuffer->GetDesc(&bufferDesc);
					int vertexCount = bufferDesc.ByteWidth / mesh->vertexStride;

					//���_���W���擾
					char* pData = reinterpret_cast<char*>(subresource.pData);
					CVector3 pos;
					for (int i = 0; i < vertexCount; i++) {
						//���W�ɃL���X�g
						pos = *reinterpret_cast<CVector3*>(pData);
						//�o�C�A�X��������B
						mBias.Mul(pos);
						//�ۑ�
						m_vertexBuffer.push_back(pos);
						//���̒��_�ցB
						pData += mesh->vertexStride;
					}

					//���_�o�b�t�@�̃|�C���^��j��(GPU�A�N�Z�X�\��)
					deviceContext->Unmap(mesh->vertexBuffer.Get(), 0);
				}
				//�C���f�b�N�X�o�b�t�@���쐬�B
				{
					//�C���f�b�N�X�o�b�t�@�̃|�C���^����ɓ����
					D3D11_MAPPED_SUBRESOURCE subresource;
					HRESULT hr = deviceContext->Map(mesh->indexBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subresource);
					if (FAILED(hr)) {
						return;
					}

					//�C���f�b�N�X�������߂�
					D3D11_BUFFER_DESC bufferDesc;
					mesh->indexBuffer->GetDesc(&bufferDesc);					
					int stride = 2;//@todo cmo�t�@�C���̓C���f�b�N�X�o�b�t�@�̃T�C�Y��2byte�Œ�B
					int indexCount = bufferDesc.ByteWidth / stride;

					//�C���f�b�N�X�擾
					unsigned short* pIndex = reinterpret_cast<unsigned short*>(subresource.pData);
					Index index;
					for (int i = 0; i < indexCount; i++) {
						index.push_back(pIndex[i]);
					}
					m_indexBuffer.push_back(index);

					//�C���f�b�N�X�o�b�t�@�̃|�C���^��j��(GPU�A�N�Z�X�\��)
					deviceContext->Unmap(mesh->indexBuffer.Get(), 0);
				}
			});
#endif
		}

	}
}
