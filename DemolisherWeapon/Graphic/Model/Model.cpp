#include "DWstdafx.h"
#include "Model.h"

namespace DemolisherWeapon {

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

		//メッシュ作成
		auto mesh = std::make_unique<SMesh>();
		mesh->m_skinFlags.reserve(tkmMesh.materials.size());

		//頂点バッファ作成
		if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
			mesh->m_vertexBuffer = std::make_unique<VertexBufferDX11>();
		}
		if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
			mesh->m_vertexBuffer = std::make_unique<VertexBufferDX12>();
		}
		mesh->m_vertexBuffer->Init(numVertex, vertexStride, (void*)&tkmMesh.vertexBuffer[0]);

		//スキンがあるか?
		auto SetSkinFlag = [&](int index) {
			if (tkmMesh.vertexBuffer[index].skinWeights.x > 0.0f) {
				//スキンがある。
				mesh->m_skinFlags.push_back(1);
			}
			else {
				//スキンなし。
				mesh->m_skinFlags.push_back(0);
			}
		};

		//インデックスバッファを作成。
		if (!tkmMesh.indexBuffer16Array.empty()) {
			//インデックスのサイズが2byte
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

				//スキンがあるかどうかを設定する。
				SetSkinFlag(tkIb.indices[0]);

				mesh->m_indexBufferArray.push_back(std::move(ib));
			}
		}
		else {
			//インデックスのサイズが4byte
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

				//スキンがあるかどうかを設定する。
				SetSkinFlag(tkIb.indices[0]);

				mesh->m_indexBufferArray.push_back(std::move(ib));
			}
		}
		//マテリアルを作成。
		mesh->m_materials.reserve(tkmMesh.materials.size());
		for (auto& tkmMat : tkmMesh.materials) {
			auto mat = std::make_unique<MaterialSetting>();
			//mat->InitFromTkmMaterila(tkmMat);//初期化
			mesh->m_materials.push_back(std::move(mat));
		}

		m_meshs[meshNo] = std::move(mesh);
	}

	void CMeshParts::Draw() {
		for (auto& mesh : m_meshs) {
			//頂点バッファを設定
			mesh->m_vertexBuffer->Attach();
			//マテリアルごとにドロー。
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//インデックスバッファを設定
				mesh->m_indexBufferArray[matNo]->Attach();

				//TODO マテリアル無しで描画してみる
				//ここでマテリアルの処理
				//ディスクリプタ設定?

				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					//トポロジーを設定
					GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					//描画
					GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->DrawIndexed(mesh->m_indexBufferArray[matNo]->GetIndexNum(), 0, 0);
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					//トポロジーを設定
					GetGraphicsEngine().GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					//描画
					GetGraphicsEngine().GetCommandList()->DrawIndexedInstanced(mesh->m_indexBufferArray[matNo]->GetIndexNum(), 1, 0, 0, 0);
				}
			}
		}

		/// <summary>
		/// //
		/// </summary>
		/// 
		/*if (m_isCreateDescriptorHeap == false) {
			//ディスクリプタヒープを作成。
			CreateDescriptorHeaps();
		}

		auto& ge12 = g_graphicsEngine->As<CGraphicsEngineDx12>();
		//レンダリングコンテキストをDx12版にダウンキャスト
		auto& rc12 = rc.As<CRenderContextDx12>();
		auto& lightMgr = ge12.GetLightManager()->As<CLightManagerDx12>();

		//メッシュごとにドロー
		//プリミティブのトポロジーはトライアングルリストのみ。
		rc12.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//定数バッファを更新する。
		SConstantBuffer cb;
		cb.mWorld = mWorld;
		cb.mView = mView;
		cb.mProj = mProj;
		m_commonConstantBuffer.Update(&cb);

		if (m_boneMatricesStructureBuffer.IsInited()) {
			//ボーン行列を更新する。
			m_boneMatricesStructureBuffer.Update(m_skeleton->GetBoneMatricesTopAddress());
		}

		int descriptorHeapNo = 0;
		for (auto& mesh : m_meshs) {
			//頂点バッファを設定。
			rc12.SetVertexBuffer(mesh->m_vertexBuffer);
			//マテリアルごとにドロー。
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//このマテリアルが貼られているメッシュの描画開始。
				mesh->m_materials[matNo]->BeginRender(rc, mesh->skinFlags[matNo]);

				auto& descriptorHeap = m_descriptorHeap.at(descriptorHeapNo);

				rc12.SetDescriptorHeap(descriptorHeap);
				descriptorHeapNo++;
				//インデックスバッファを設定。
				auto& ib = mesh->m_indexBufferArray[matNo];
				rc12.SetIndexBuffer(ib);

				//ドロー。
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
			DW_WARNING_BOX(true, "この関数はtkmファイルのロードが完了してから呼び出してください。");
			return;
		}
		m_meshParts.InitFromTkmFile(m_tkmFile);
	}
	void CModel::Draw()
	{
		m_meshParts.Draw();
	}

}