#include "DWstdafx.h"
#include "MeshCollider.h"
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"

namespace DemolisherWeapon {

	MeshCollider::MeshCollider()
	{
	}
	MeshCollider::~MeshCollider()
	{
	}

	/*!
	 * @brief	CSkinModelからメッシュコライダーを生成。
	 *@param[in]	model		スキンモデル。
	 */
	void MeshCollider::CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix)
	{
		//バイアスの作成
		CMatrix mBias, mBiasScr;
		CoordinateSystemBias::GetBias(mBias, mBiasScr, model.GetFBXUpAxis(), model.GetFBXCoordinateSystem());
		mBias.Mul(mBiasScr, mBias);
		if (offsetMatrix != nullptr) {
			mBias.Mul(mBias, (*offsetMatrix));
		}

		m_stridingMeshInterface = std::make_unique<btTriangleIndexVertexArray>();

		if (model.IsLoadedModel()) {
			model.FindMeshCModel([&](const auto& mesh) {
				//頂点バッファを作成
				VertexBufferPtr vertexBuffer = std::make_unique<VertexBuffer>();
				CVector3 pos;
				for (int i = 0; i < mesh->m_vertexNum; i++) {
					pos = mesh->m_vertexData[i].position;
					mBias.Mul(pos);//バイアスをかける
					vertexBuffer->push_back(pos);
				}
				m_vertexBufferArray.push_back(std::move(vertexBuffer));

				//インデックスバッファを作成
				IndexBufferPtr indexBuffer = std::make_unique<IndexBuffer>();
				for (auto& indD : mesh->m_indexDataArray) {
					for (auto& ind : indD) {
						indexBuffer->emplace_back(ind);
					}
				}
				m_indexBufferArray.push_back(std::move(indexBuffer));

				//インデックスメッシュを作成
				CreateIndexMesh(m_indexBufferArray.back().get(), m_vertexBufferArray.back().get());
				}
			);
		}

		if (model.IsLoadedDirectXTKModel()) {
#ifdef DW_DX11
			ID3D11DeviceContext* deviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();
			model.FindMesh([&](const auto& mesh) {
				//頂点バッファを作成。
				{
					D3D11_MAPPED_SUBRESOURCE subresource;
					HRESULT hr = deviceContext->Map(mesh->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subresource);
					if (FAILED(hr)) {
						return;
					}
					D3D11_BUFFER_DESC bufferDesc;
					mesh->vertexBuffer->GetDesc(&bufferDesc);
					int vertexCount = bufferDesc.ByteWidth / mesh->vertexStride;
					char* pData = reinterpret_cast<char*>(subresource.pData);
					VertexBufferPtr vertexBuffer = std::make_unique<VertexBuffer>();
					CVector3 pos;
					for (int i = 0; i < vertexCount; i++) {
						pos = *reinterpret_cast<CVector3*>(pData);
						//バイアスをかける。
						mBias.Mul(pos);
						vertexBuffer->push_back(pos);
						//次の頂点へ。
						pData += mesh->vertexStride;
					}
					//頂点バッファをアンロック
					deviceContext->Unmap(mesh->vertexBuffer.Get(), 0);
					m_vertexBufferArray.push_back(std::move(vertexBuffer));
				}
				//インデックスバッファを作成。
				{
					D3D11_MAPPED_SUBRESOURCE subresource;
					//インデックスバッファをロック。
					HRESULT hr = deviceContext->Map(mesh->indexBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subresource);
					if (FAILED(hr)) {
						return;
					}
					D3D11_BUFFER_DESC bufferDesc;
					mesh->indexBuffer->GetDesc(&bufferDesc);
					//@todo cmoファイルはインデックスバッファのサイズは2byte固定。
					IndexBufferPtr indexBuffer = std::make_unique<IndexBuffer>();
					int stride = 2;
					int indexCount = bufferDesc.ByteWidth / stride;
					unsigned short* pIndex = reinterpret_cast<unsigned short*>(subresource.pData);
					for (int i = 0; i < indexCount; i++) {
						indexBuffer->push_back(pIndex[i]);
					}
					//インデックスバッファをアンロック。
					deviceContext->Unmap(mesh->indexBuffer.Get(), 0);
					m_indexBufferArray.push_back(std::move(indexBuffer));
				}

				//この辺使わなくていいのか?
				//https://social.msdn.microsoft.com/Forums/ja-JP/cb07dac2-67f9-4f4a-b779-c29af14705d4/fbx1239812514124871252312487125401247912398indexbuffer12364274912412?forum=xnagameja
				//https://github.com/microsoft/DirectXTK/wiki/ModelMeshPart
				/*mesh->indexCount;
				mesh->startIndex;
				mesh->vertexOffset;
				mesh->vertexStride;*/

				//インデックスメッシュを作成。
				CreateIndexMesh(m_indexBufferArray.back().get(), m_vertexBufferArray.back().get());
				}
			);
#endif
		}

		m_meshShape = std::make_unique<btBvhTriangleMeshShape>(m_stridingMeshInterface.get(), true);

		//btAdjustInternalEdgeContactsのやつ
		m_triInfomap = std::make_unique<btTriangleInfoMap>();
		btGenerateInternalEdgeInfo(m_meshShape.get(), m_triInfomap.get());
	}

	void MeshCollider::CreateIndexMesh(IndexBuffer* ib, VertexBuffer* vb) {
		btIndexedMesh indexedMesh;
		indexedMesh.m_numTriangles = (int)ib->size() / 3;
		indexedMesh.m_triangleIndexBase = (unsigned char*)(&ib->front());
		indexedMesh.m_triangleIndexStride = 12;
		indexedMesh.m_numVertices = (int)vb->size();
		indexedMesh.m_vertexBase = (unsigned char*)(&vb->front());
		indexedMesh.m_vertexStride = sizeof(CVector3);
		m_stridingMeshInterface->addIndexedMesh(indexedMesh);
	}

}
