#include "DWstdafx.h"
#include "ReyTracingEngine.h"

namespace DemolisherWeapon {

	namespace {
		ID3D12Resource* CreateBuffer(
			ID3D12Device5* pDevice,
			uint64_t size,
			D3D12_RESOURCE_FLAGS flags,
			D3D12_RESOURCE_STATES initState,
			const D3D12_HEAP_PROPERTIES& heapProps)
		{
			D3D12_RESOURCE_DESC bufDesc = {};
			bufDesc.Alignment = 0;
			bufDesc.DepthOrArraySize = 1;
			bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufDesc.Flags = flags;
			bufDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufDesc.Height = 1;
			bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			bufDesc.MipLevels = 1;
			bufDesc.SampleDesc.Count = 1;
			bufDesc.SampleDesc.Quality = 0;
			bufDesc.Width = size;

			ID3D12Resource* pBuffer;
			pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer));
			return pBuffer;
		}
	}

	void BLASBuffer::Init(ID3D12GraphicsCommandList4* commandList, const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instances)
	{
		for (auto& instance : instances) {
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
			inputs.NumDescs = 1;
			inputs.pGeometryDescs = &instance->geometoryDesc;
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

			auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
			d3dDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

			AccelerationStructureBuffers asbuffer;
			asbuffer.pScratch = CreateBuffer(
				d3dDevice,
				info.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COMMON,
				kDefaultHeapProps);

			asbuffer.pResult = CreateBuffer(
				d3dDevice,
				info.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				kDefaultHeapProps);

			// Create the bottom-level AS
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
			asDesc.Inputs = inputs;
			asDesc.DestAccelerationStructureData = asbuffer.pResult->GetGPUVirtualAddress();
			asDesc.ScratchAccelerationStructureData = asbuffer.pScratch->GetGPUVirtualAddress();
			commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

			//���C�g���[�V���O�A�N�Z�����[�V�����\���̃r���h�����҂��̃o���A������B
			D3D12_RESOURCE_BARRIER uavBarrier = {};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = asbuffer.pResult;
			commandList->ResourceBarrier(1, &uavBarrier);

			m_bottomLevelASBuffers.push_back(std::move(asbuffer));
		}
	}

	void TLASBuffer::Init(
		ID3D12GraphicsCommandList4* commandList,
		const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instances,
		const std::vector< AccelerationStructureBuffers>& bottomLevelASBuffers
	)
	{
		uint64_t tlasSize;
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();

		size_t numInstance = instances.size();
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		inputs.NumDescs = (UINT)numInstance;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		d3dDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		/*if (update) {
			//�X�V�H
		}
		else*/ {
		//�V�K�H
			m_topLevelASBuffers.pScratch = CreateBuffer(d3dDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, kDefaultHeapProps);
			m_topLevelASBuffers.pResult = CreateBuffer(d3dDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);
			m_topLevelASBuffers.pInstanceDesc = CreateBuffer(
				d3dDevice,
				sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numInstance,
				D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
				kUploadHeapProps
			);
			tlasSize = info.ResultDataMaxSizeInBytes;
		}

		//Map the instance desc buffer
		D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
		m_topLevelASBuffers.pInstanceDesc->Map(0, nullptr, (void**)&instanceDescs);
		ZeroMemory(instanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numInstance);

		CMatrix mRot;
		mRot.MakeRotationX(CMath::PI * -0.5f);
		mRot.Transpose();

		for (int i = 0; i < numInstance; i++) {
			instanceDescs[i].InstanceID = i;
			instanceDescs[i].InstanceContributionToHitGroupIndex = (int)enHitGroup_Num * i + enHitGroup_PBRCameraRay;
			instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			instanceDescs[i].AccelerationStructure = bottomLevelASBuffers[i].pResult->GetGPUVirtualAddress();
			memcpy(instanceDescs[i].Transform, &mRot, sizeof(instanceDescs[i].Transform));
			instanceDescs[i].InstanceMask = 0xFF;
		}

		m_topLevelASBuffers.pInstanceDesc->Unmap(0, nullptr);

		//TopLevelAS���쐬�B
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
		asDesc.Inputs = inputs;
		asDesc.Inputs.InstanceDescs = m_topLevelASBuffers.pInstanceDesc->GetGPUVirtualAddress();
		asDesc.DestAccelerationStructureData = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		asDesc.ScratchAccelerationStructureData = m_topLevelASBuffers.pScratch->GetGPUVirtualAddress();

		/*if (update)
		{
			asDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
			asDesc.SourceAccelerationStructureData = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		}*/
		commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

		//���C�g���[�V���O�A�N�Z�����[�V�����\���̃r���h�����҂��̃o���A������B
		D3D12_RESOURCE_BARRIER uavBarrier = {};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = m_topLevelASBuffers.pResult;
		commandList->ResourceBarrier(1, &uavBarrier);
	}
	void TLASBuffer::RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo)
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
		//TLAS���f�B�X�N���v�^�q�[�v�ɓo�^�B
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, descriptorHandle);
	}

	namespace {
		Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc, const wchar_t* name)
		{
			auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
			Microsoft::WRL::ComPtr<ID3DBlob> pSigBlob;
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
			HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
			if (FAILED(hr))
			{
				MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "�G���[", MB_OK);
				std::abort();
			}
			Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSig;
			d3dDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig));
			pRootSig->SetName(name);
			return pRootSig;
		}
	}
	//�T�u�I�u�W�F�N�g�쐬�̃w���p�[�B
	namespace BuildSubObjectHelper {
		/// <summary>
		/// ���[�J����V�O�l�`���̃T�u�I�u�W�F�N�g�쐬�̃w���p�[�\���́B
		/// </summary>
		struct LocalRootSignatureSubobject {
			LocalRootSignatureSubobject()
			{
			}
			void Init(const D3D12_ROOT_SIGNATURE_DESC& desc, const wchar_t* name)
			{
				pRootSig = CreateRootSignature(desc, name);
				pInterface = pRootSig.Get();
				subobject.pDesc = &pInterface;
				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			}
			Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSig;
			ID3D12RootSignature* pInterface = nullptr;
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// ExportAssociation�̃T�u�I�u�W�F�N�g�쐬�̃w���p�[�\���́B
		/// </summary>
		struct ExportAssociationSubobject
		{
			void Init(const WCHAR* exportNames[], uint32_t exportCount, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
			{
				association.NumExports = exportCount;
				association.pExports = exportNames;
				association.pSubobjectToAssociate = pSubobjectToAssociate;

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				subobject.pDesc = &association;
			}
			D3D12_STATE_SUBOBJECT subobject = {};
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association = {};
		};
		struct ShaderConfigSubobject
		{
			void Init(uint32_t maxAttributeSizeInBytes, uint32_t maxPayloadSizeInBytes)
			{
				shaderConfig.MaxAttributeSizeInBytes = maxAttributeSizeInBytes;
				shaderConfig.MaxPayloadSizeInBytes = maxPayloadSizeInBytes;

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
				subobject.pDesc = &shaderConfig;
			}
			D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// �p�C�v���C���ݒ�̃T�u�I�u�W�F�N�g�쐬�̃w���p�[�\���́B
		/// </summary>
		struct PipelineConfigSubobject
		{
			PipelineConfigSubobject()
			{
				config.MaxTraceRecursionDepth = MAX_TRACE_RECURSION_DEPTH;

				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
				subobject.pDesc = &config;
			}

			D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// �O���[�o�����[�g�V�O�l�`���̃T�u�I�u�W�F�N�g�쐬�̃w���p�[�\���́B
		/// </summary>
		struct GlobalRootSignatureSubobject
		{
			GlobalRootSignatureSubobject()
			{
				pRootSig = CreateRootSignature({}, L"GlobalRootSignature");
				pInterface = pRootSig.Get();
				subobject.pDesc = &pInterface;
				subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
			}
			Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSig;
			ID3D12RootSignature* pInterface = nullptr;
			D3D12_STATE_SUBOBJECT subobject = {};
		};
		/// <summary>
		/// �q�b�g�O���[�v�T�u�I�u�W�F�N�g
		/// </summary>
		struct HitGroupSubObject
		{
			HitGroupSubObject() {}
			void Init(const SHitGroup& hitgroup)
			{
				desc = {};
				desc.AnyHitShaderImport = hitgroup.anyHitShaderName;
				desc.ClosestHitShaderImport = hitgroup.chsHitShaderName;
				desc.HitGroupExport = hitgroup.name;

				subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
				subObject.pDesc = &desc;
			}
			D3D12_HIT_GROUP_DESC desc;
			D3D12_STATE_SUBOBJECT subObject;
		};
	};
	ReyTracingPSO::RootSignatureDesc ReyTracingPSO::CreateRayGenRootSignatureesc()
	{
		// Create the root-signature
		RootSignatureDesc desc;
		desc.range.resize(3);
		// gOutput
		desc.range[0].BaseShaderRegister = 0;
		desc.range[0].NumDescriptors = 1;
		desc.range[0].RegisterSpace = 0;
		desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		//UAV�f�B�X�N���v�^���n�܂�z��ԍ�
		desc.range[0].OffsetInDescriptorsFromTableStart = 0;//TODO m_srvUavCbvHeap->GetOffsetUAVDescriptorFromTableStart();

		// gRtScene
		desc.range[1].BaseShaderRegister = 0;
		desc.range[1].NumDescriptors = (int)ESRV_OneEntry::eNumRayGenerationSRV;
		desc.range[1].RegisterSpace = 0;
		desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		desc.range[1].OffsetInDescriptorsFromTableStart = 1;

		desc.range[2].BaseShaderRegister = 0;
		desc.range[2].NumDescriptors = 1;
		desc.range[2].RegisterSpace = 0;
		desc.range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		desc.range[2].OffsetInDescriptorsFromTableStart = 0;

		desc.rootParams.resize(1);
		desc.rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.rootParams[0].DescriptorTable.NumDescriptorRanges = (UINT)desc.range.size();
		desc.rootParams[0].DescriptorTable.pDescriptorRanges = desc.range.data();

		// Create the desc
		desc.desc.NumParameters = 1;
		desc.desc.pParameters = desc.rootParams.data();
		desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		return desc;
	}
	ReyTracingPSO::RootSignatureDesc ReyTracingPSO::CreatePBRMatterialHitRootSignatureDesc()
	{
		RootSignatureDesc desc;

		enum ERange {
			eRange_SRV,		//SRV
			eRange_Sampler,	//�T���v��
			eRange_Num,		//�͈͂̐��B
		};
		desc.range.resize(eRange_Num);

		desc.rootParams.resize(eHitShaderDescriptorTable_Num);

		desc.range[eRange_SRV].BaseShaderRegister = 0;
		desc.range[eRange_SRV].NumDescriptors = (int)ESRV_OneEntry::eNum;
		desc.range[eRange_SRV].RegisterSpace = 0;
		desc.range[eRange_SRV].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		desc.range[eRange_SRV].OffsetInDescriptorsFromTableStart = 0;

		desc.range[eRange_Sampler].BaseShaderRegister = 0;
		desc.range[eRange_Sampler].NumDescriptors = 1;
		desc.range[eRange_Sampler].RegisterSpace = 0;
		desc.range[eRange_Sampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		desc.range[eRange_Sampler].OffsetInDescriptorsFromTableStart = 0;

		desc.rootParams[eHitShaderDescriptorTable_SRV_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.rootParams[eHitShaderDescriptorTable_SRV_CBV].DescriptorTable.NumDescriptorRanges = 1;
		desc.rootParams[eHitShaderDescriptorTable_SRV_CBV].DescriptorTable.pDescriptorRanges = &desc.range[0];

		desc.rootParams[eHitShaderDescriptorTable_Sampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.rootParams[eHitShaderDescriptorTable_Sampler].DescriptorTable.NumDescriptorRanges = 1;
		desc.rootParams[eHitShaderDescriptorTable_Sampler].DescriptorTable.pDescriptorRanges = &desc.range[1];


		desc.desc.NumParameters = static_cast<UINT>(desc.rootParams.size());
		desc.desc.pParameters = desc.rootParams.data();
		desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		return desc;
	}

	void ReyTracingPSO::Init()
	{
		using namespace BuildSubObjectHelper;

		std::array<D3D12_STATE_SUBOBJECT, 14> subobjects;
		uint32_t index = 0;

		//DXIL���C�u�������쐬�B
		//���C�g���[�V���O�p�̃V�F�[�_�[�����[�h�B
		Shader raytraceShader;
		raytraceShader.LoadRaytracing(L"Assets/shader/sample.fx");

		D3D12_EXPORT_DESC libExport[eShader_Num];
		for (int i = 0; i < eShader_Num; i++) {
			libExport[i].Name = shaderDatas[i].entryPointName;
			libExport[i].ExportToRename = nullptr;
			libExport[i].Flags = D3D12_EXPORT_FLAG_NONE;
		};

		D3D12_DXIL_LIBRARY_DESC dxLibdesc;
		auto pBlob = raytraceShader.GetCompiledDxcBlob();
		dxLibdesc.DXILLibrary.pShaderBytecode = pBlob->GetBufferPointer();
		dxLibdesc.DXILLibrary.BytecodeLength = pBlob->GetBufferSize();
		dxLibdesc.NumExports = ARRAYSIZE(libExport);
		dxLibdesc.pExports = libExport;

		subobjects[index].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobjects[index].pDesc = &dxLibdesc;

		index++;

		//HitGroup�̃T�u�I�u�W�F�N�g���쐬�B
		std::vector< HitGroupSubObject> hitGroupSOs;
		hitGroupSOs.resize(eHitGroup_Num);
		for (int i = 0; i < eHitGroup_Num; i++) {
			hitGroupSOs[i].Init(hitGroups[i]);
			subobjects[index++] = hitGroupSOs[i].subObject; // 1 Hit Group
		}

		auto BuildAndRegistRootSignatureAndAssSubobjectFunc = [&](
			LocalRootSignatureSubobject& rsSO, ExportAssociationSubobject& ass, ELocalRootSignature eRS, const WCHAR* exportNames[]
			) {
				if (eRS == eLocalRootSignature_Raygen) {
					rsSO.Init(CreateRayGenRootSignatureesc().desc, L"RayGenRootSignature");
				}
				if (eRS == eLocalRootSignature_PBRMaterialHit) {
					rsSO.Init(CreatePBRMatterialHitRootSignatureDesc().desc, L"PBRMaterialHitGenRootSignature");
				}
				if (eRS == eLocalRootSignature_Empty) {
					D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
					emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
					rsSO.Init(emptyDesc, L"EmptyRootSignature");
				}
				subobjects[index] = rsSO.subobject;
				uint32_t rgSOIndex = index++;

				int useRootSignature = 0;
				for (auto& shaderData : shaderDatas) {
					if (shaderData.useLocalRootSignature == eRS) {
						//�����͎w�肳�ꂽ���[�g�V�O�l�`�����g���B
						exportNames[useRootSignature] = shaderData.entryPointName;
						useRootSignature++;
					}
				}
				ass.Init(exportNames, useRootSignature, &(subobjects[rgSOIndex]));
				subobjects[index++] = ass.subobject;
		};
		//���[�g�V�O�l�`���ƃV�F�[�_�[�̊֘A�t�����s���T�u�I�u�W�F�N�g������Ă����B
		LocalRootSignatureSubobject rayGenSignatureSO, modelSignatureSO, emptySignatureSO;
		ExportAssociationSubobject rayGenAssSO, modelAssSO, emptyAssSO;
		const WCHAR* rayGenExportName[eShader_Num];
		const WCHAR* modelExportName[eShader_Num];
		const WCHAR* emptyExportName[eShader_Num];

		BuildAndRegistRootSignatureAndAssSubobjectFunc(rayGenSignatureSO, rayGenAssSO, eLocalRootSignature_Raygen, rayGenExportName);
		BuildAndRegistRootSignatureAndAssSubobjectFunc(modelSignatureSO, modelAssSO, eLocalRootSignature_PBRMaterialHit, modelExportName);
		BuildAndRegistRootSignatureAndAssSubobjectFunc(emptySignatureSO, emptyAssSO, eLocalRootSignature_Empty, emptyExportName);


		// Payload�̃T�C�Y�ƈ����̐��͂Ƃ肠�����Œ�ŁE�E�E�B��Ō����B
		ShaderConfigSubobject shaderConfig;
		struct RayPayload
		{
			CVector4 color;
			CVector4 reflectionColor;
			CVector4 hit_depth;
		};
		shaderConfig.Init(sizeof(float) * 2, sizeof(RayPayload));
		subobjects[index] = shaderConfig.subobject; // 

		uint32_t shaderConfigIndex = index++;
		ExportAssociationSubobject configAssociationSO;
		const WCHAR* entryPointNames[eShader_Num];
		for (int i = 0; i < eShader_Num; i++) {
			entryPointNames[i] = shaderDatas[i].entryPointName;
		}
		configAssociationSO.Init(entryPointNames, eShader_Num, &subobjects[shaderConfigIndex]);
		subobjects[index++] = configAssociationSO.subobject;

		// �p�C�v���C���ݒ�̃T�u�I�u�W�F�N�g���쐬�B
		PipelineConfigSubobject config;
		subobjects[index++] = config.subobject;

		// �O���[�o�����[�g�V�O�l�`���̃T�u�I�u�W�F�N�g���쐬�B
		GlobalRootSignatureSubobject root;
		m_emptyRootSignature = root.pRootSig;
		subobjects[index++] = root.subobject;

		// Create the state
		D3D12_STATE_OBJECT_DESC desc;
		desc.NumSubobjects = index;
		desc.pSubobjects = subobjects.data();
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
		auto hr = d3dDevice->CreateStateObject(&desc, IID_PPV_ARGS(&m_pipelineState));
		if (FAILED(hr)) {
			MessageBox(nullptr, "�p�C�v���C���X�e�[�g�̍쐬�Ɏ��s���܂����B\n", "�G���[", MB_OK);
			std::abort();
		}
	}

	void ReyTracingWorld::RegistGeometry(CModel& model) {
		model.FindMesh([&](const std::unique_ptr<SModelMesh>& mesh){
			const auto& vertexBufferView = dynamic_cast<VertexBufferDX12*>(mesh->m_vertexBuffer.get())->GetView();
			for (int i = 0; i < mesh->m_materials.size(); i++) {
				const auto& indexBufferView = dynamic_cast<IndexBufferDX12*>(mesh->m_indexBufferArray[i].get())->GetView();

				D3D12_RAYTRACING_GEOMETRY_DESC desc;
				memset(&desc, 0, sizeof(desc));

				desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				desc.Triangles.Transform3x4 = 0;
				desc.Triangles.VertexBuffer.StartAddress = vertexBufferView.BufferLocation;
				desc.Triangles.VertexBuffer.StrideInBytes = vertexBufferView.StrideInBytes;
				desc.Triangles.VertexCount = vertexBufferView.SizeInBytes / vertexBufferView.StrideInBytes;
				desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
				desc.Triangles.IndexBuffer = indexBufferView.BufferLocation;
				desc.Triangles.IndexCount = (UINT)mesh->m_indexDataArray[i].size();
				desc.Triangles.IndexFormat = indexBufferView.Format;

				std::unique_ptr<ReyTracingInstanceData> instance = std::make_unique<ReyTracingInstanceData>();
				instance->geometoryDesc = desc;
				//instance->m_material = mesh.m_materials[i];
				//instance->m_vertexBufferRWSB.Init(mesh.m_vertexBuffer, false);
				//instance->m_indexBufferRWSB.Init(*mesh.m_indexBufferArray[i], false);

				//�z��ɃC���X�^���X�ǉ�
				m_instances.emplace_back(std::move(instance));
			}
		});
	}
	void ReyTracingWorld::CommitRegistGeometry(ID3D12GraphicsCommandList4* commandList) {
		//BLAS���\�z�B
		m_blasBuffer.Init(commandList, m_instances);
		//TLAS���\�z�B
		m_topLevelASBuffers.Init(commandList, m_instances, m_blasBuffer.Get());
	}

	void ShaderTable::CountupNumGeyGenAndMissAndHitShader()
	{
		//�e�V�F�[�_�[�̐��𒲍��B
		m_numRayGenShader = 0;
		m_numMissShader = 0;
		m_numHitShader = 0;
		for (auto& shaderData : shaderDatas) {
			if (shaderData.category == eShaderCategory_RayGenerator) m_numRayGenShader++;
			if (shaderData.category == eShaderCategory_Miss) m_numMissShader++;
			if (shaderData.category == eShaderCategory_ClosestHit) m_numHitShader++;
		}
	}

#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

	void ShaderTable::CalcShaderTableEntrySize()
	{
		//�V�F�[�_�[�e�[�u���ɓo�^�����f�[�^�̂P�v�f���̃T�C�Y���v�Z�B
		//�V�F�[�_�[���ʎq�B
		m_shaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		//�f�B�X�N���v�^�e�[�u���̃A�h���X�B
		m_shaderTableEntrySize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE) * eHitShaderDescriptorTable_Num; // The hit shader
		//�A���C�����g�����낦��B
		m_shaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, m_shaderTableEntrySize);
	}
	void ShaderTable::Init(
		const ReyTracingWorld& world,
		const ReyTracingPSO& pso
		//const DescriptorHeaps& descriptorHeaps
	)
	{
		//�e�V�F�[�_�[�̐����J�E���g����B
		CountupNumGeyGenAndMissAndHitShader();

		//�V�F�[�_�[�e�[�u���ɓo�^�����f�[�^�̂P�v�f���̃T�C�Y���v�Z�B
		CalcShaderTableEntrySize();

		//�V�F�[�_�[�e�[�u���̃T�C�Y���v�Z�B
		int shaderTableSize = m_shaderTableEntrySize * (m_numRayGenShader + m_numMissShader + (m_numHitShader * world.GetNumInstance()));

		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
		//�V�F�[�_�[�e�[�u���p�̃o�b�t�@���쐬�B
		m_shaderTable = CreateBuffer(d3dDevice, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);

		//�o�b�t�@���V�X�e���������Ƀ}�b�v����B
		uint8_t* pData;
		m_shaderTable->Map(0, nullptr, (void**)&pData);

		Microsoft::WRL::ComPtr< ID3D12StateObjectProperties > pRtsoProps;
		pso.QueryInterface(pRtsoProps);

		uint8_t* pCurret = pData;

		auto ds_size_cbv_srv_uav = GetGraphicsEngine().GetDX12().GetSrvsDescriptorSize();
		auto hitGroup_pbrCameraRaySrvHeapStride =
			ds_size_cbv_srv_uav * (int)ESRV_OneEntry::eNum;

		const auto& srvUavCbvDescriptorHeapStart = GetGraphicsEngine().GetDX12().GetSrvsDescriptorHeapStart();
		const auto& samplerDescriptorHeapStart = GetGraphicsEngine().GetDX12().GetSamplerDescriptorHeapStart();

		uint64_t hitGroup_pbrCameraRaySrvHeapStart = srvUavCbvDescriptorHeapStart.ptr + ds_size_cbv_srv_uav;
		//�V�F�[�_�[�e�[�u���ɃV�F�[�_�[��o�^����֐��B
		auto RegistShaderTblFunc = [&](const ShaderData& shaderData, EShaderCategory registCategory, ReyTracingInstanceData* instance) {
			if (shaderData.category == registCategory) {
				//�܂��V�F�[�_�[ID��ݒ肷��B
				void* pShaderId = nullptr;
				if (registCategory == eShaderCategory_ClosestHit) {
					pShaderId = pRtsoProps->GetShaderIdentifier(hitGroups[shaderData.hitgroup].name);
				}
				else {
					pShaderId = pRtsoProps->GetShaderIdentifier(shaderData.entryPointName);
				}
				memcpy(pCurret, pShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
				uint8_t* pDst = pCurret + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
				if (shaderData.useLocalRootSignature == eLocalRootSignature_Raygen) {
					//�f�t�H���g�̃��[�g�V�O�l�`�����g�p����ꍇ�A�V�F�[�_�[ID�̌��Ƀf�B�X�N���v�^�q�[�v�ւ̃A�h���X��ݒ肷��B
					*(uint64_t*)(pDst) = srvUavCbvDescriptorHeapStart.ptr;
				}
				if (shaderData.useLocalRootSignature == eLocalRootSignature_PBRMaterialHit) {
					//SRV_CBV�̃f�B�X�N���v�^�q�[�v				
					*(uint64_t*)(pDst) = hitGroup_pbrCameraRaySrvHeapStart;
					pDst += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
					*(uint64_t*)(pDst) = samplerDescriptorHeapStart.ptr;

				}
				//���B
				pCurret += m_shaderTableEntrySize;
			}
		};
		// ���C�W�F�l���[�V�����V�F�[�_�[���e�[�u���ɓo�^���Ă����B
		for (auto& shader : shaderDatas) {
			RegistShaderTblFunc(shader, eShaderCategory_RayGenerator, nullptr);
		}
		// �����ă~�X�V�F�[�_�[�B
		for (auto& shader : shaderDatas) {
			RegistShaderTblFunc(shader, eShaderCategory_Miss, nullptr);
		}
		//�Ō�Ƀq�b�g�V�F�[�_�[�B�q�b�g�V�F�[�_�[�̓q�b�g�V�F�[�_�[�̐��@�~�@�C���X�^���X�̐������o�^����B
		world.QueryInstances([&](ReyTracingInstanceData& instance) {
			for (auto& shader : shaderDatas) {
				RegistShaderTblFunc(shader, eShaderCategory_ClosestHit, &instance);
			};
			hitGroup_pbrCameraRaySrvHeapStart += hitGroup_pbrCameraRaySrvHeapStride;	//��
		});

		//Unmap
		m_shaderTable->Unmap(0, nullptr);
	}

	/*void DescriptorHeaps::Init(
		World& world,
		GPUBuffer& outputBuffer,
		ConstantBuffer& rayGeneCB
	)
	{

		//���C�g���̏o�͐���f�B�X�N���v�^�q�[�v�ɓo�^����B
		m_srvUavCbvHeap.RegistUnorderAccessResource(0, outputBuffer);

		m_srvUavCbvHeap.RegistConstantBuffer(0, rayGeneCB);

		int regNo = 0;
		world.QueryInstances([&](Instance& instance)
			{
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eTLAS,
					world.GetTLASBuffer()
				);
				//�A���x�h�}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eAlbedoMap,
					instance.m_material->GetAlbedoMap()
				);
				//�@���}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eNormalMap,
					instance.m_material->GetNormalMap()
				);
				//�X�y�L�����}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eSpecularMap,
					instance.m_material->GetSpecularMap()
				);

				//���t���N�V�����}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eReflectionMap,
					instance.m_material->GetReflectionMap()
				);

				//���܃}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eRefractionMap,
					instance.m_material->GetRefractionMap()
				);
				//���_�o�b�t�@���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eVertexBuffer,
					instance.m_vertexBufferRWSB
				);
				//�C���f�b�N�X�o�b�t�@���f�B�X�N���v�^�q�[�v�ɓo�^�B
				m_srvUavCbvHeap.RegistShaderResource(
					regNo + (int)ESRV_OneEntry::eIndexBuffer,
					instance.m_indexBufferRWSB
				);
				regNo += (int)ESRV_OneEntry::eNum;

			});

		//�T���v���X�e�[�g�̈����͉��B
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		//�T���v���X�e�[�g���f�B�X�N���v�^�q�[�v�ɓo�^����B
		m_samplerDescriptorHeap.RegistSamplerDesc(0, samplerDesc);
		m_samplerDescriptorHeap.CommitSamperHeap();
		m_srvUavCbvHeap.Commit();
	}*/

	void ReyTracingEngine::Dispatch(ID3D12GraphicsCommandList4* commandList)
	{
		//�萔�X�V
		CBStructure cam;
		cam.pos = GetMainCamera()->GetPos();
		cam.mRot = GetMainCamera()->GetRotMatrix();
		cam.aspect = GetMainCamera()->GetAspect();
		cam.fNear = GetMainCamera()->GetNear();
		cam.fFar = GetMainCamera()->GetFar();
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
		raytraceDesc.Width =  GetGraphicsEngine().Get3DFrameBuffer_W();//
		raytraceDesc.Height = GetGraphicsEngine().Get3DFrameBuffer_H();
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
		/*const DescriptorHeap* descriptorHeaps[] = {
			&m_descriptorHeaps.GetSrvUavCbvDescriptorHeap(),
			&m_descriptorHeaps.GetSamplerDescriptorHeap()
		};
		rc.SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);*/

		ID3D12DescriptorHeap* m_descriptorHeaps[2];
		for (int i = 0; i < 2; i++) {
			m_descriptorHeaps[i];
		}
		commandList->SetDescriptorHeaps(2, m_descriptorHeaps);

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
		//g_graphicsEngine->CopyToFrameBuffer(rc, m_outputResource.Get());*/

	}
	void ReyTracingEngine::CommitRegistGeometry(ID3D12GraphicsCommandList4* commandList)
	{
		//����Ȃ��Ȃ�?
		//�����_�[�^�[�Q�b�g�Ƃ��ݒ�
		//g_graphicsEngine->BeginRender();

		//AS�̍\�z
		m_world.CommitRegistGeometry(commandList);

		//�V�F�[�_�[���\�[�X(�萔�o�b�t�@�Ƃ�)�쐬�B
		CreateShaderResources();

		//�e�탊�\�[�X���f�B�X�N���v�^�q�[�v�ɓo�^����B
		//m_descriptorHeaps.Init(m_world, m_outputResource, m_rayGenerationCB);

		//PSO���쐬�B
		m_pipelineStateObject.Init();

		//�V�F�[�_�[�e�[�u�����쐬�B
		m_shaderTable.Init(m_world, m_pipelineStateObject);

		//����Ȃ��Ȃ�?
		//�R�}���h���s�Ƃ�present
		//g_graphicsEngine->EndRender();
	}
	void ReyTracingEngine::CreateShaderResources()
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();

		//�o�͐�o�b�t�@�̍쐬
		{
			//�o�b�N�o�b�t�@�ɍ��킹�Ă�c
			auto backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;// m_deviceResources->GetBackBufferFormat();
			auto width = GetGraphicsEngine().Get3DFrameBuffer_W();
			auto height = GetGraphicsEngine().Get3DFrameBuffer_W();

			//���\�[�X�쐬
			// Create the output resource. The dimensions and format should match the swap-chain.
			auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			DirectX::ThrowIfFailed(
				d3dDevice->CreateCommittedResource(
					&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)
				)
			);
			m_raytracingOutput->SetName(L"RaytracingOutputBuffer");//���\�[�X�ɖ��O����

			//UAV���b�s
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			m_raytracingOutputResourceUAVGpuDescriptor = GetGraphicsEngine().GetDX12().CreateUAV(m_raytracingOutput.Get(), UAVDesc);
		}

		//���C�W�F�l���[�V�����p�̒萔�o�b�t�@�B
		CBStructure cam;
		cam.pos = GetMainCamera()->GetPos();
		cam.mRot = GetMainCamera()->GetRotMatrix();
		cam.aspect = GetMainCamera()->GetAspect();
		cam.fNear = GetMainCamera()->GetNear();
		cam.fFar = GetMainCamera()->GetFar();
		m_rayGenerationCB.Init(sizeof(CBStructure), &cam);
	}
}