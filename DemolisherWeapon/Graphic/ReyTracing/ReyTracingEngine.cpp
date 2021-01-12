#include "DWstdafx.h"
#include "ReyTracingEngine.h"

namespace DemolisherWeapon {		

	std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator ReyTracingWorld::RegisterModel(CModel& model, const CMatrix* worldMatrix) {
		m_isUpdated = true;

		//ジオメトリのインデックス取得
		int geometoryIndex = model.GetRayTracingWorldStartIndex();
		//ジオメトリ初期化済みか?
		bool isInitedGeometory = geometoryIndex >= 0;
		if (!isInitedGeometory) {
			//ジオメトリのレイトレ用初期化
			model.InitRayTracingVertex();
		}

		int startIndex = -1;
		bool isFirst = true;
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator startItr;

		model.FindMesh([&](const std::unique_ptr<SModelMesh>& mesh){
			const auto& vertexBufferView = dynamic_cast<VertexBufferDX12*>(mesh->m_vertexBufferDXR.get())->GetView();
			for (int i = 0; i < mesh->m_materials.size(); i++) {
				//ジオメトリ作成
				if (!isInitedGeometory) {
					const auto& indexBufferView = dynamic_cast<IndexBufferDX12*>(mesh->m_indexBufferArray[i].get())->GetView();
					
					//GEOMETRY_DESC作る
					D3D12_RAYTRACING_GEOMETRY_DESC desc;
					memset(&desc, 0, sizeof(desc));
					desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
					desc.Triangles.Transform3x4 = 0;
					desc.Triangles.VertexBuffer.StartAddress = vertexBufferView.BufferLocation;
					desc.Triangles.VertexBuffer.StrideInBytes = vertexBufferView.StrideInBytes;
					desc.Triangles.VertexCount = vertexBufferView.SizeInBytes / vertexBufferView.StrideInBytes;
					desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
					if (!mesh->m_materials[i]->GetMaterialData().GetIsAlpha()) {
						desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
					}
					desc.Triangles.IndexBuffer = indexBufferView.BufferLocation;
					desc.Triangles.IndexCount = (UINT)mesh->m_indexDataArray[i].size();
					desc.Triangles.IndexFormat = indexBufferView.Format;

					std::unique_ptr<ReyTracingGeometoryData> geometory = std::make_unique<ReyTracingGeometoryData>();
					geometory->m_geometoryDesc = desc;
					geometory->m_vertexBufferRWSB.Init(*dynamic_cast<VertexBufferDX12*>(mesh->m_vertexBufferDXR.get()), false);
					geometory->m_indexBufferRWSB.Init(*dynamic_cast<IndexBufferDX12*>(mesh->m_indexBufferArray[i].get()), false);
				
					//配列に追加
					m_geometories.emplace_back(std::move(geometory));

					//ジオメトリの開始インデックス設定
					if (geometoryIndex < 0) {
						geometoryIndex = (int)m_geometories.size() - 1;
						startIndex = geometoryIndex;
					}
				}

				//インスタンス作成
				std::unique_ptr<ReyTracingInstanceData> instance = std::make_unique<ReyTracingInstanceData>();				
				instance->m_geometory = m_geometories[geometoryIndex].get();
				instance->m_material = mesh->m_materials[i]->GetMaterialData().GetUsingMaterialSettingPtr();
				instance->m_worldMatrix = worldMatrix;
				//配列にインスタンス追加
				m_instances.emplace_back(std::move(instance));

				if (isFirst) {
					startItr = m_instances.end();
					startItr--;
					isFirst = false;
				}

				geometoryIndex++;
			}
		});

		//ジオメトリのインデックス記録
		if (startIndex >= 0) {
			model.SetRayTracingWorldIndex(startIndex, geometoryIndex - 1);
		}

		//インスタンス配列の開始イテレーター返す
		return startItr;
	}
	void ReyTracingWorld::UnregisterModel(std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator startItr, const CModel& model) {
#ifndef DW_MASTER
		int geometoryIndex = model.GetRayTracingWorldStartIndex();
		if (geometoryIndex < 0) {
			DW_WARNING_MESSAGE(true, "ReyTracingWorld::UnregisterModel() 登録されていないインスタンスを登録解除しようとしています。\n")
				return;
		}
#endif
		//指定回数イテレーターまわして削除
		int num = model.GetRayTracingWorldGeometoryNum();
		auto endItr = startItr;
		std::advance(endItr, num);
		m_instances.erase(startItr, endItr);

		//更新しました
		m_isUpdated = true;
	}
	/*void ReyTracingWorld::UnregisterModel(CModel& model, const CMatrix* worldMatrix) {
		int geometoryIndex = model.GetRayTracingWorldStartIndex();
		if (geometoryIndex < 0) {
			DW_WARNING_MESSAGE(true, "ReyTracingWorld::UnregisterModel() 登録されていないインスタンスを登録解除しようとしています。\n")
			return;
		}

		//インスタンス削除
		auto itr = m_instances.end();

		model.FindMesh([&](const std::unique_ptr<SModelMesh>& mesh) {
			for (int i = 0; i < mesh->m_materials.size(); i++) {
				itr = std::remove_if(
					m_instances.begin(),
					itr,
					[&](const auto& ins) {
						return ins->m_geometory == m_geometories[geometoryIndex].get() && ins->m_worldMatrix == worldMatrix;
					}
				);
				geometoryIndex++;
			}
			}
		);

		m_instances.erase(
			itr,
			m_instances.end()
		);
		
		m_isUpdated = true;
	}*/
	void ReyTracingWorld::CommitRegisterGeometry(ID3D12GraphicsCommandList4* commandList) {
		//BLASを構築。
		m_blasBuffer.Init(commandList, m_geometories);
		//TLASを構築。
		m_topLevelASBuffers.Init(commandList, m_instances, m_blasBuffer.Get(), false);

		m_isUpdated = false;
	}
	void ReyTracingWorld::UpdateTLAS(ID3D12GraphicsCommandList4* commandList) {
		//TLAS更新
		m_topLevelASBuffers.Init(commandList, m_instances, m_blasBuffer.Get(), true);
	}	

	void RayTracingEngine::Dispatch(ID3D12GraphicsCommandList4* commandList)
	{
		if (!m_isCommit || m_world.GetNumInstance() == 0) {
			return;//コミットしてない
		}

		//定数更新
		ReyTracingCBStructure cam;
		cam.pos = GetMainCamera()->GetPos();
		cam.mRot = GetMainCamera()->GetRotMatrix();
		cam.fNear = GetMainCamera()->GetNear();
		cam.fFar = GetMainCamera()->GetFar();
		cam.fov = GetMainCamera()->GetFOV();
		m_rayGenerationCB.Update(&cam);

		//出力バッファをバリア
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = m_raytracingOutput.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		commandList->ResourceBarrier(1, &barrier);

		//レイトレに必要な情報をここから記述
		D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
		raytraceDesc.Width =  (UINT)GetGraphicsEngine().GetFrameBuffer_W();//TODO
		raytraceDesc.Height = (UINT)GetGraphicsEngine().GetFrameBuffer_H();
		raytraceDesc.Depth = 1;

		//シェーダーテーブルの情報取得
		auto shaderTableEntrySize = m_shaderTable.GetShaderTableEntrySize();
		auto numRayGenShader = m_shaderTable.GetNumRayGenShader();
		auto numMissShader = m_shaderTable.GetNumMissShader();
		auto numHitShader = m_shaderTable.GetNumHitShader();

		// レイ生成シェーダーのシェーダーテーブルの開始アドレスとサイズを設定。
		raytraceDesc.RayGenerationShaderRecord.StartAddress = m_shaderTable.GetGPUVirtualAddress();
		raytraceDesc.RayGenerationShaderRecord.SizeInBytes = shaderTableEntrySize;

		// ミスシェーダーのシェーダーテーブルの開始アドレスとサイズを設定。
		size_t missOffset = numRayGenShader * shaderTableEntrySize;
		raytraceDesc.MissShaderTable.StartAddress = m_shaderTable.GetGPUVirtualAddress() + missOffset;
		raytraceDesc.MissShaderTable.StrideInBytes = shaderTableEntrySize;
		raytraceDesc.MissShaderTable.SizeInBytes = shaderTableEntrySize * numMissShader;

		// ヒットグループシェーダーの開始アドレスとサイズを設定。
		size_t hitOffset = (numRayGenShader + numMissShader) * shaderTableEntrySize;
		raytraceDesc.HitGroupTable.StartAddress = m_shaderTable.GetGPUVirtualAddress() + hitOffset;
		raytraceDesc.HitGroupTable.StrideInBytes = shaderTableEntrySize;
		raytraceDesc.HitGroupTable.SizeInBytes = shaderTableEntrySize * numHitShader * m_world.GetNumInstance();

		// グローバルルートシグネチャを設定。
		commandList->SetComputeRootSignature(m_pipelineStateObject.GetGlobalRootSignature());

		// Dispatch
		//グローバルルートシグネチチャに登録されているディスクリプタヒープを登録する。
		ID3D12DescriptorHeap* descriptorHeaps[] = {
			m_descriptorHeap.GetSRVHeap(),
			m_descriptorHeap.GetSamplerHeap()
		};
		commandList->SetDescriptorHeaps(2, descriptorHeaps);

		//レイトレ用のパイプラインステートオブジェクトを設定
		commandList->SetPipelineState1(m_pipelineStateObject.Get());

		//レイトレー
		commandList->DispatchRays(&raytraceDesc);

		//出力用バッファのバリア
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = m_raytracingOutput.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		commandList->ResourceBarrier(1, &barrier);

		//レイトレの結果をフレームバッファに書き戻す。
		{
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				GetGraphicsEngine().GetDX12().GetCurrentRenderTarget(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_COPY_DEST);
			commandList->ResourceBarrier(1, &barrier);

			commandList->CopyResource(GetGraphicsEngine().GetDX12().GetCurrentRenderTarget(), m_raytracingOutput.Get());

			D3D12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
				GetGraphicsEngine().GetDX12().GetCurrentRenderTarget(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &barrier2);
		}
	}
	void RayTracingEngine::Update(ID3D12GraphicsCommandList4* commandList)
	{
		//インスタンスなし
		if (m_world.GetNumInstance() == 0) {
			return;

		}

		//インスタンスの追加削除がない場合
		if (!m_world.GetIsUpdated()) {
			if (m_isCommit) {
				m_world.UpdateTLAS(commandList);//TLASのみ更新
				//m_descriptorHeap.Update(m_world);
			}
			return;//更新なし
		}

		//ASの構築
		m_world.CommitRegisterGeometry(commandList);

		//シェーダーリソース(定数バッファとか)作成。
		if (!m_isCommit) {
			CreateShaderResources();
		}

		//各種リソースをディスクリプタヒープに登録する。
		if (!m_isCommit) {
			m_descriptorHeap.Init(m_world, m_rayGenerationCB, m_raytracingOutputResourceUAVCpuDescriptor);
		}
		else {
			m_descriptorHeap.Update(m_world);
		}

		//PSOを作成。
		if (!m_isCommit) {
			m_pipelineStateObject.Init(&m_descriptorHeap);
		}

		//シェーダーテーブルを作成。
		m_shaderTable.Init(m_world, m_pipelineStateObject, m_descriptorHeap);

		m_isCommit = true;
	}
	void RayTracingEngine::CreateShaderResources()
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();

		//出力先バッファの作成
		{
			//バックバッファに合わせてる… TODO
			auto backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;// m_deviceResources->GetBackBufferFormat();
			UINT width = (UINT)GetGraphicsEngine().GetFrameBuffer_W();
			UINT height = (UINT)GetGraphicsEngine().GetFrameBuffer_H();

			//リソース作成
			// Create the output resource. The dimensions and format should match the swap-chain.
			auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			DirectX::ThrowIfFailed(
				d3dDevice->CreateCommittedResource(
					&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_raytracingOutput)
				)
			);
			m_raytracingOutput->SetName(L"RaytracingOutputBuffer");//リソースに名前つける

			//UAV作るッピ
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			auto[gpu,cpu] = GetGraphicsEngine().GetDX12().CreateUAV(m_raytracingOutput.Get(), UAVDesc);
			m_raytracingOutputResourceUAVGpuDescriptor = gpu;
			m_raytracingOutputResourceUAVCpuDescriptor = cpu;
		}

		//レイジェネレーション用の定数バッファ。
		ReyTracingCBStructure cam;
		cam.pos = GetMainCamera()->GetPos();
		cam.mRot = GetMainCamera()->GetRotMatrix();
		cam.fNear = GetMainCamera()->GetNear();
		cam.fFar = GetMainCamera()->GetFar();
		cam.fov = GetMainCamera()->GetFOV();
		m_rayGenerationCB.Init(sizeof(ReyTracingCBStructure), &cam);
	}
}