#include "DWstdafx.h"
#include "ReyTracingEngine.h"

namespace DemolisherWeapon {		

	std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator ReyTracingWorld::RegisterModel(CModel& model, const CMatrix* worldMatrix) {
		m_isUpdated = true;

		//�W�I���g���̃C���f�b�N�X�擾
		int geometoryIndex = model.GetRayTracingWorldStartIndex();
		//�W�I���g���������ς݂�?
		bool isInitedGeometory = geometoryIndex >= 0;
		if (!isInitedGeometory) {
			//�W�I���g���̃��C�g���p������
			model.InitRayTracingVertex();
		}

		int startIndex = -1;
		bool isFirst = true;
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator startItr;

		model.FindMesh([&](const std::unique_ptr<SModelMesh>& mesh){
			const auto& vertexBufferView = dynamic_cast<VertexBufferDX12*>(mesh->m_vertexBufferDXR.get())->GetView();
			for (int i = 0; i < mesh->m_materials.size(); i++) {
				//�W�I���g���쐬
				if (!isInitedGeometory) {
					const auto& indexBufferView = dynamic_cast<IndexBufferDX12*>(mesh->m_indexBufferArray[i].get())->GetView();
					
					//GEOMETRY_DESC���
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
				
					//�z��ɒǉ�
					m_geometories.emplace_back(std::move(geometory));

					//�W�I���g���̊J�n�C���f�b�N�X�ݒ�
					if (geometoryIndex < 0) {
						geometoryIndex = (int)m_geometories.size() - 1;
						startIndex = geometoryIndex;
					}
				}

				//�C���X�^���X�쐬
				std::unique_ptr<ReyTracingInstanceData> instance = std::make_unique<ReyTracingInstanceData>();				
				instance->m_geometory = m_geometories[geometoryIndex].get();
				instance->m_material = mesh->m_materials[i]->GetMaterialData().GetUsingMaterialSettingPtr();
				instance->m_worldMatrix = worldMatrix;
				//�z��ɃC���X�^���X�ǉ�
				m_instances.emplace_back(std::move(instance));

				if (isFirst) {
					startItr = m_instances.end();
					startItr--;
					isFirst = false;
				}

				geometoryIndex++;
			}
		});

		//�W�I���g���̃C���f�b�N�X�L�^
		if (startIndex >= 0) {
			model.SetRayTracingWorldIndex(startIndex, geometoryIndex - 1);
		}
		startItr->get()->m_geometoryNum = model.GetRayTracingWorldGeometoryNum();

		//�C���X�^���X�z��̊J�n�C�e���[�^�[�Ԃ�
		return startItr;
	}
	void ReyTracingWorld::UnregisterModel(std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator startItr) {
//#ifndef DW_MASTER
//		int geometoryIndex = model.GetRayTracingWorldStartIndex();
//		if (geometoryIndex < 0) {
//			DW_WARNING_MESSAGE(true, "ReyTracingWorld::UnregisterModel() �o�^����Ă��Ȃ��C���X�^���X��o�^�������悤�Ƃ��Ă��܂��B\n")
//				return;
//		}
//#endif
		//�w��񐔃C�e���[�^�[�܂킵�č폜
		int num = startItr->get()->m_geometoryNum;
		auto endItr = startItr;
		std::advance(endItr, num);
		m_instances.erase(startItr, endItr);

		//�X�V���܂���
		m_isUpdated = true;
	}	
	void ReyTracingWorld::CommitRegisterGeometry(ID3D12GraphicsCommandList4* commandList) {
		//BLAS���\�z�B
		m_blasBuffer.Init(commandList, m_geometories);
		//TLAS���\�z�B
		m_topLevelASBuffers.Init(commandList, m_instances, m_blasBuffer.Get(), false);

		m_isUpdated = false;
	}
	void ReyTracingWorld::UpdateTLAS(ID3D12GraphicsCommandList4* commandList) {
		//TLAS�X�V
		m_topLevelASBuffers.Init(commandList, m_instances, m_blasBuffer.Get(), true);
	}	

	void RayTracingEngine::Dispatch(ID3D12GraphicsCommandList4* commandList)
	{
		if (!m_isCommit || m_world.GetNumInstance() == 0) {
			return;//�R�~�b�g���ĂȂ�
		}

		//�萔�X�V
		ReyTracingCBStructure cam;
		cam.pos = GetMainCamera()->GetPos();
		cam.mRot = GetMainCamera()->GetRotMatrix();
		cam.fNear = GetMainCamera()->GetNear();
		cam.fFar = GetMainCamera()->GetFar();
		cam.fov = GetMainCamera()->GetFOV();
		m_rayGenerationCB.Update(&cam);

		//�o�̓o�b�t�@���o���A
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = m_raytracingOutput.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		commandList->ResourceBarrier(1, &barrier);

		//���C�g���ɕK�v�ȏ�����������L�q
		D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
		raytraceDesc.Width =  (UINT)GetGraphicsEngine().GetFrameBuffer_W();//TODO
		raytraceDesc.Height = (UINT)GetGraphicsEngine().GetFrameBuffer_H();
		raytraceDesc.Depth = 1;

		//�V�F�[�_�[�e�[�u���̏��擾
		auto shaderTableEntrySize = m_shaderTable.GetShaderTableEntrySize();
		auto numRayGenShader = m_shaderTable.GetNumRayGenShader();
		auto numMissShader = m_shaderTable.GetNumMissShader();
		auto numHitShader = m_shaderTable.GetNumHitShader();

		// ���C�����V�F�[�_�[�̃V�F�[�_�[�e�[�u���̊J�n�A�h���X�ƃT�C�Y��ݒ�B
		raytraceDesc.RayGenerationShaderRecord.StartAddress = m_shaderTable.GetGPUVirtualAddress();
		raytraceDesc.RayGenerationShaderRecord.SizeInBytes = shaderTableEntrySize;

		// �~�X�V�F�[�_�[�̃V�F�[�_�[�e�[�u���̊J�n�A�h���X�ƃT�C�Y��ݒ�B
		size_t missOffset = numRayGenShader * shaderTableEntrySize;
		raytraceDesc.MissShaderTable.StartAddress = m_shaderTable.GetGPUVirtualAddress() + missOffset;
		raytraceDesc.MissShaderTable.StrideInBytes = shaderTableEntrySize;
		raytraceDesc.MissShaderTable.SizeInBytes = shaderTableEntrySize * numMissShader;

		// �q�b�g�O���[�v�V�F�[�_�[�̊J�n�A�h���X�ƃT�C�Y��ݒ�B
		size_t hitOffset = (numRayGenShader + numMissShader) * shaderTableEntrySize;
		raytraceDesc.HitGroupTable.StartAddress = m_shaderTable.GetGPUVirtualAddress() + hitOffset;
		raytraceDesc.HitGroupTable.StrideInBytes = shaderTableEntrySize;
		raytraceDesc.HitGroupTable.SizeInBytes = shaderTableEntrySize * numHitShader * m_world.GetNumInstance();

		// �O���[�o�����[�g�V�O�l�`����ݒ�B
		commandList->SetComputeRootSignature(m_pipelineStateObject.GetGlobalRootSignature());

		// Dispatch
		//�O���[�o�����[�g�V�O�l�`�`���ɓo�^����Ă���f�B�X�N���v�^�q�[�v��o�^����B
		ID3D12DescriptorHeap* descriptorHeaps[] = {
			m_descriptorHeap.GetSRVHeap(),
			m_descriptorHeap.GetSamplerHeap()
		};
		commandList->SetDescriptorHeaps(2, descriptorHeaps);

		//���C�g���p�̃p�C�v���C���X�e�[�g�I�u�W�F�N�g��ݒ�
		commandList->SetPipelineState1(m_pipelineStateObject.Get());

		//���C�g���[
		commandList->DispatchRays(&raytraceDesc);

		//�o�͗p�o�b�t�@�̃o���A
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = m_raytracingOutput.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		commandList->ResourceBarrier(1, &barrier);

		//���C�g���̌��ʂ��t���[���o�b�t�@�ɏ����߂��B
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
		//�C���X�^���X�Ȃ�
		if (m_world.GetNumInstance() == 0) {
			return;

		}

		//�C���X�^���X�̒ǉ��폜���Ȃ��ꍇ
		if (!m_world.GetIsUpdated()) {
			if (m_isCommit) {
				m_world.UpdateTLAS(commandList);//TLAS�̂ݍX�V
				//m_descriptorHeap.Update(m_world);
			}
			return;//�X�V�Ȃ�
		}

		//AS�̍\�z
		m_world.CommitRegisterGeometry(commandList);

		//�V�F�[�_�[���\�[�X(�萔�o�b�t�@�Ƃ�)�쐬�B
		if (!m_isCommit) {
			CreateShaderResources();
		}

		//�e�탊�\�[�X���f�B�X�N���v�^�q�[�v�ɓo�^����B
		if (!m_isCommit) {
			m_descriptorHeap.Init(m_world, m_rayGenerationCB, m_raytracingOutputResourceUAVCpuDescriptor);
		}
		else {
			m_descriptorHeap.Update(m_world);
		}

		//PSO���쐬�B
		if (!m_isCommit) {
			m_pipelineStateObject.Init(&m_descriptorHeap);
		}

		//�V�F�[�_�[�e�[�u�����쐬�B
		m_shaderTable.Init(m_world, m_pipelineStateObject, m_descriptorHeap);

		m_isCommit = true;
	}
	void RayTracingEngine::CreateShaderResources()
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();

		//�o�͐�o�b�t�@�̍쐬
		{
			//�o�b�N�o�b�t�@�ɍ��킹�Ă�c TODO
			auto backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;// m_deviceResources->GetBackBufferFormat();
			UINT width = (UINT)GetGraphicsEngine().GetFrameBuffer_W();
			UINT height = (UINT)GetGraphicsEngine().GetFrameBuffer_H();

			//���\�[�X�쐬
			// Create the output resource. The dimensions and format should match the swap-chain.
			auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			DirectX::ThrowIfFailed(
				d3dDevice->CreateCommittedResource(
					&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_raytracingOutput)
				)
			);
			m_raytracingOutput->SetName(L"RaytracingOutputBuffer");//���\�[�X�ɖ��O����

			//UAV���b�s
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			auto[gpu,cpu] = GetGraphicsEngine().GetDX12().CreateUAV(m_raytracingOutput.Get(), UAVDesc);
			m_raytracingOutputResourceUAVGpuDescriptor = gpu;
			m_raytracingOutputResourceUAVCpuDescriptor = cpu;
		}

		//���C�W�F�l���[�V�����p�̒萔�o�b�t�@�B
		ReyTracingCBStructure cam;
		cam.pos = GetMainCamera()->GetPos();
		cam.mRot = GetMainCamera()->GetRotMatrix();
		cam.fNear = GetMainCamera()->GetNear();
		cam.fFar = GetMainCamera()->GetFar();
		cam.fov = GetMainCamera()->GetFOV();
		m_rayGenerationCB.Init(sizeof(ReyTracingCBStructure), &cam);
	}
}