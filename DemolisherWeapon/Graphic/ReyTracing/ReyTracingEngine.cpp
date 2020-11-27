#include "DWstdafx.h"
#include "ReyTracingEngine.h"

namespace DemolisherWeapon {

	namespace {
		void CreateBuffer(
			ID3D12Device5* pDevice,
			uint64_t size,
			D3D12_RESOURCE_FLAGS flags,
			D3D12_RESOURCE_STATES initState,
			const D3D12_HEAP_PROPERTIES& heapProps,
			Microsoft::WRL::ComPtr<ID3D12Resource>& output,
			const wchar_t* name
		)
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

			pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(output.ReleaseAndGetAddressOf()));
			output->SetName(name);
		}
	}

	void BLASBuffer::Init(
		ID3D12GraphicsCommandList4* commandList,
		const std::vector<std::unique_ptr<ReyTracingGeometoryData>>& geometories
	)
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
		
		for (auto& geometory : geometories) {
			//���łɍ\�z�ς�
			if (geometory->m_pResultBLAS) {
				continue;
			}

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
			inputs.NumDescs = 1;
			inputs.pGeometryDescs = &geometory->m_geometoryDesc;
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
			d3dDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

			AccelerationStructureBuffers asbuffer;
			CreateBuffer(
				d3dDevice,
				info.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COMMON,
				kDefaultHeapProps,
				asbuffer.pScratch,
				L"BLAS��pScratch"
			);
			CreateBuffer(
				d3dDevice,
				info.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				kDefaultHeapProps,
				asbuffer.pResult,
				L"BLAS��pResult"
			);

			// Create the bottom-level AS
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
			asDesc.Inputs = inputs;
			asDesc.DestAccelerationStructureData = asbuffer.pResult->GetGPUVirtualAddress();
			asDesc.ScratchAccelerationStructureData = asbuffer.pScratch->GetGPUVirtualAddress();
			commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

			//���C�g���[�V���O�A�N�Z�����[�V�����\���̃r���h�����҂��̃o���A������B
			D3D12_RESOURCE_BARRIER uavBarrier = {};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = asbuffer.pResult.Get();
			commandList->ResourceBarrier(1, &uavBarrier);

			m_bottomLevelASBuffers.push_back(std::move(asbuffer));
			geometory->m_pResultBLAS = m_bottomLevelASBuffers.back().pResult.Get();
		}
	}

	void TLASBuffer::Init(
		ID3D12GraphicsCommandList4* commandList,
		const std::vector<std::unique_ptr<ReyTracingInstanceData>>& instances,
		const std::vector<AccelerationStructureBuffers>& bottomLevelASBuffers,
		bool update
	)
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();
		size_t numInstance = instances.size();

		//uint64_t tlasSize;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = (UINT)numInstance;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		d3dDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		if (update){
			//�X�V
			D3D12_RESOURCE_BARRIER uavBarrier = {};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = m_topLevelASBuffers.pResult.Get();
			commandList->ResourceBarrier(1, &uavBarrier);
		}
		else {
			//�V�K
			CreateBuffer(
				d3dDevice,
				info.ScratchDataSizeInBytes, 
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				kDefaultHeapProps, 
				m_topLevelASBuffers.pScratch,
				L"TLAS��pScratch"
			);
			CreateBuffer(
				d3dDevice,
				info.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				kDefaultHeapProps,
				m_topLevelASBuffers.pResult,
				L"TLAS��pResult"
			);
			CreateBuffer(
				d3dDevice,
				sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numInstance,
				D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
				kUploadHeapProps,
				m_topLevelASBuffers.pInstanceDesc,
				L"TLAS��pInstanceDesc"
			);
			//tlasSize = info.ResultDataMaxSizeInBytes;
		}

		//Map the instance desc buffer
		D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
		HRESULT hr = m_topLevelASBuffers.pInstanceDesc->Map(0, nullptr, (void**)&instanceDescs);
		if (FAILED(hr)) {
			HRESULT hr2 = d3dDevice->GetDeviceRemovedReason();
			std::abort();
		}
		ZeroMemory(instanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numInstance);		

		for (int i = 0; i < numInstance; i++) {			
			instanceDescs[i].InstanceID = i;
			instanceDescs[i].InstanceContributionToHitGroupIndex = (int)eHitGroup_Num * i + eHitGroup_PBRCameraRay;//�g�p����q�b�g�O���[�v
			instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			instanceDescs[i].AccelerationStructure = instances[i]->m_geometory->m_pResultBLAS->GetGPUVirtualAddress();
			instanceDescs[i].InstanceMask = 0xFF;//�[�����ƃ��C�ɓ�����Ȃ� //����Ŕ�\���Ƃ����?

			//�g�����X�t�H�[���s��ݒ�
			CMatrix mTrans = *instances[i]->m_worldMatrix;
			mTrans.Transpose();
			memcpy(instanceDescs[i].Transform, &mTrans, sizeof(instanceDescs[i].Transform));			
		}

		m_topLevelASBuffers.pInstanceDesc->Unmap(0, nullptr);

		//TopLevelAS���쐬�B
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
		asDesc.Inputs = inputs;
		asDesc.Inputs.InstanceDescs = m_topLevelASBuffers.pInstanceDesc->GetGPUVirtualAddress();
		asDesc.DestAccelerationStructureData = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		asDesc.ScratchAccelerationStructureData = m_topLevelASBuffers.pScratch->GetGPUVirtualAddress();
		if (update)
		{
			asDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
			asDesc.SourceAccelerationStructureData = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		}
		commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

		//���C�g���[�V���O�A�N�Z�����[�V�����\���̃r���h�����҂��̃o���A������B
		D3D12_RESOURCE_BARRIER uavBarrier = {};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = m_topLevelASBuffers.pResult.Get();
		commandList->ResourceBarrier(1, &uavBarrier);
	}
	void TLASBuffer::CreateShaderResourceView()
	{
		//TLAS���f�B�X�N���v�^�q�[�v�ɓo�^�B
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();

		auto [gpu,cpu] = GetGraphicsEngine().GetDX12().CreateSRV(nullptr, &srvDesc);
		m_GPUdescriptorHandle = gpu;
		m_CPUdescriptorHandle = cpu;
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
	ReyTracingPSO::RootSignatureDesc ReyTracingPSO::CreateRayGenRootSignatureDesc()
	{
		// Create the root-signature
		RootSignatureDesc desc;
		desc.range.resize(3);
		// gOutput
		desc.range[0].BaseShaderRegister = 0;//�͈͓��̃x�[�X�V�F�[�_�[���W�X�^�B���Ƃ��΁A�V�F�[�_�[���\�[�X�r���[�iSRV�j�̏ꍇ�A3�́u�Fregister�it3�j;�v�Ƀ}�b�v����܂��BHLSL�ŁB
		desc.range[0].NumDescriptors = 1;		
		desc.range[0].RegisterSpace = 0;//���Ƃ��΁ASRV�̏ꍇ�ABaseShaderRegister�����o�[�̐����̗���g�����邱�Ƃɂ��A5�́u�Fregister�it3�Aspace5�j;�v�Ƀ}�b�v����܂��BHLSL�ŁB
		desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;		
		desc.range[0].OffsetInDescriptorsFromTableStart = m_descriptorHeap->GetOffsetUAVDescriptorFromTableStart();

		// gRtScene
		desc.range[1].BaseShaderRegister = 0;
		desc.range[1].NumDescriptors = (int)ESRV_OneEntry::eNumRayGenerationSRV;
		desc.range[1].RegisterSpace = 0;
		desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		desc.range[1].OffsetInDescriptorsFromTableStart = m_descriptorHeap->GetOffsetSRVDescriptorFromTableStart();

		desc.range[2].BaseShaderRegister = 0;
		desc.range[2].NumDescriptors = 1;
		desc.range[2].RegisterSpace = 0;
		desc.range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		desc.range[2].OffsetInDescriptorsFromTableStart = m_descriptorHeap->GetOffsetCBVDescriptorFromTableStart();

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

	void ReyTracingPSO::Init(const ReyTracingDescriptorHeap* DH)
	{
		m_descriptorHeap = DH;
		
		using namespace BuildSubObjectHelper;

		std::array<D3D12_STATE_SUBOBJECT, 14> subobjects;
		uint32_t index = 0;

		//DXIL���C�u�������쐬�B
		//���C�g���[�V���O�p�̃V�F�[�_�[�����[�h�B
		Shader raytraceShader;
		raytraceShader.LoadRaytracing(L"preset/shader/raytrace.fx");

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
					rsSO.Init(CreateRayGenRootSignatureDesc().desc, L"RayGenRootSignature");
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

		/// <summary>
		/// raytrace.fx : RayPayload
		/// </summary>
		struct RayPayload
		{
			CVector3 color;
			int hit;
			int depth;
		};
		uint32_t attributesSize = sizeof(float) * 2;// BuiltInTriangleIntersectionAttributes�̃T�C�Y

		shaderConfig.Init(attributesSize, sizeof(RayPayload));
		subobjects[index] = shaderConfig.subobject; 

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

	void ReyTracingWorld::RegisterModel(CModel& model, const CMatrix* worldMatrix) {
		m_isUpdated = true;

		//�W�I���g���̃C���f�b�N�X�擾
		int geometoryIndex = model.GetRayTracingWorldStartIndex();
		//�W�I���g���������ς݂�?
		bool isInitedGeometory = geometoryIndex >= 0;
		if (!isInitedGeometory) {
			//�W�I���g���̃��C�g���p������
			model.InitRayTracingVertex();
		}

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
						model.SetRayTracingWorldStartIndex(geometoryIndex);
					}
				}

				//�C���X�^���X�쐬
				std::unique_ptr<ReyTracingInstanceData> instance = std::make_unique<ReyTracingInstanceData>();				
				instance->m_geometory = m_geometories[geometoryIndex].get();
				instance->m_material = mesh->m_materials[i]->GetMaterialData().GetUsingMaterialSettingPtr();
				instance->m_worldMatrix = worldMatrix;
				//�z��ɃC���X�^���X�ǉ�
				m_instances.emplace_back(std::move(instance));

				geometoryIndex++;
			}
		});
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
		const ReyTracingPSO& pso,
		const ReyTracingDescriptorHeap& descriptorHeaps
	)
	{
		//�e�V�F�[�_�[�̐����J�E���g����B
		CountupNumGeyGenAndMissAndHitShader();

		//�V�F�[�_�[�e�[�u���ɓo�^�����f�[�^�̂P�v�f���̃T�C�Y���v�Z�B
		CalcShaderTableEntrySize();

		//�V�F�[�_�[�e�[�u���̃T�C�Y���v�Z�B
		int shaderTableSize = m_shaderTableEntrySize * (m_numRayGenShader + m_numMissShader + (m_numHitShader * world.GetNumInstance()));

		//�V�F�[�_�[�e�[�u���p�̃o�b�t�@���쐬�B
		CreateBuffer(
			GetGraphicsEngine().GetD3D12Device(),
			shaderTableSize, 
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ, 
			kUploadHeapProps,
			m_shaderTable,
			L"ShaderTableBuffer"
		);

		//�o�b�t�@���V�X�e���������Ƀ}�b�v����B
		uint8_t* pData;
		m_shaderTable->Map(0, nullptr, (void**)&pData);

		Microsoft::WRL::ComPtr< ID3D12StateObjectProperties > pRtsoProps;
		pso.QueryInterface(pRtsoProps);

		uint8_t* pCurret = pData;

		auto ds_size_cbv_srv_uav = descriptorHeaps.GetSrvsDescriptorSize();
		auto hitGroup_pbrCameraRaySrvHeapStride =
			ds_size_cbv_srv_uav * (int)ESRV_OneEntry::eNum;

		const auto& srvUavCbvDescriptorHeapStart = descriptorHeaps.GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
		const auto& samplerDescriptorHeapStart = descriptorHeaps.GetSamplerHeap()->GetGPUDescriptorHandleForHeapStart();

		//���[�J�����[�g�V�O�l�`��"eLocalRootSignature_PBRMaterialHit"��SRV�J�n�ʒu
		uint64_t hitGroup_pbrCameraRaySrvHeapStart = srvUavCbvDescriptorHeapStart.ptr + ds_size_cbv_srv_uav * descriptorHeaps.GetOffsetSRVDescriptorFromTableStart();
		
		//�V�F�[�_�[�e�[�u���ɃV�F�[�_�[��o�^����֐��B
		auto RegistShaderTblFunc = [&](const ShaderData& shaderData, EShaderCategory registCategory, ReyTracingInstanceData* instance) 
		{
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

				//�A�h���X�i�߂�
				uint8_t* pDst = pCurret + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

				//�e�f�B�X�N���v�^�q�[�v�̎n�܂�A�h���X���Z�b�g?

				if (shaderData.useLocalRootSignature == eLocalRootSignature_Raygen) {
					//�f�t�H���g�̃��[�g�V�O�l�`�����g�p����ꍇ�A�V�F�[�_�[ID�̌��Ƀf�B�X�N���v�^�q�[�v�ւ̃A�h���X��ݒ肷��B
					*(uint64_t*)(pDst) = srvUavCbvDescriptorHeapStart.ptr;
				}

				if (shaderData.useLocalRootSignature == eLocalRootSignature_PBRMaterialHit) {
					//SRV_CBV�̃f�B�X�N���v�^�q�[�v				
					*(uint64_t*)(pDst) = hitGroup_pbrCameraRaySrvHeapStart;
					//��
					pDst += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
					//�T���v���[�̃f�B�X�N���v�^�q�[�v
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

	namespace {
		constexpr int SRVS_DESC_NUM = 1000000;
		constexpr int SAMPLER_DESC_NUM = 3;
	}

	void ReyTracingDescriptorHeap::Init(ReyTracingWorld& world, ConstantBuffer<ReyTracingCBStructure>& cb, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle) 
	{
		auto device = GetGraphicsEngine().GetD3D12Device();
		
		//CBV_SRV_UAV�p�̃f�X�N���v�^�q�[�v�쐬
		m_srvsDescriptorSize = GetGraphicsEngine().GetDX12().CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SRVS_DESC_NUM, true, m_srvsDescriptorHeap);
		//�T���v���[�p�̃f�X�N���v�^�q�[�v�쐬
		m_samplerDescriptorSize = GetGraphicsEngine().GetDX12().CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SAMPLER_DESC_NUM, true, m_samplerDescriptorHeap);

		int srvIndex = 0;

		//CBV�J�n�ʒu
		m_cbvStartNum = srvIndex;
		//�萔		
		device->CopyDescriptorsSimple(
			1,
			m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),//dest
			cb.GetCPUDescriptorHandle(),//src
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);
		srvIndex++;

		//UAV�J�n�ʒu
		m_uavStartNum = srvIndex;
		//UAV�̏�
		device->CopyDescriptorsSimple(
			1,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex, m_srvsDescriptorSize),//dest
			uavHandle,//src
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);
		srvIndex++;

		//SRV�J�n�ʒu
		m_srvStartNum = srvIndex;
		//SRV
		srvIndex = Update(world);

		//�T���v��
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
		//�쐬
		device->CreateSampler(&samplerDesc, m_samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	int ReyTracingDescriptorHeap::Update(ReyTracingWorld& world) {
		auto device = GetGraphicsEngine().GetD3D12Device();
		int srvIndex = m_srvStartNum;
		
		//TLAS��SRV�쐬
		world.GetTLASBuffer().CreateShaderResourceView();

		//SRV
		world.QueryInstances([&](ReyTracingInstanceData& instance)
			{
				//TLAS�̓o�^
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eTLAS, m_srvsDescriptorSize),//dest
					world.GetTLASBuffer().GetCPUDescriptorHandle(),//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);

				//�A���x�h�}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eAlbedoMap, m_srvsDescriptorSize),//dest
					instance.m_material->GetAlbedoTextureData().CPUdescriptorHandle,//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);
				//�@���}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				//�����̓_�~�[
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eNormalMap, m_srvsDescriptorSize),//dest
					GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
					//instance.m_material->GetUsingMaterialSetting().GetNormalTextureData().CPUdescriptorHandle,//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);
				//�X�y�L�����}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				//�����̓_�~�[
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eSpecularMap, m_srvsDescriptorSize),//dest
					GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
					//instance.m_material->GetUsingMaterialSetting().GetLightingTextureData().CPUdescriptorHandle,//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);

				//���t���N�V�����}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				//�����̓_�~�[
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eReflectionMap, m_srvsDescriptorSize),//dest
					GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);
				//���܃}�b�v���f�B�X�N���v�^�q�[�v�ɓo�^�B
				//�����̓_�~�[
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eRefractionMap, m_srvsDescriptorSize),//dest
					GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);

				//���_�o�b�t�@���f�B�X�N���v�^�q�[�v�ɓo�^�B
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eVertexBuffer, m_srvsDescriptorSize),//dest
					instance.m_geometory->m_vertexBufferRWSB.GetCPUDescriptorHandle(),//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);
				//�C���f�b�N�X�o�b�t�@���f�B�X�N���v�^�q�[�v�ɓo�^�B
				device->CopyDescriptorsSimple(
					1,
					CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eIndexBuffer, m_srvsDescriptorSize),//dest
					instance.m_geometory->m_indexBufferRWSB.GetCPUDescriptorHandle(),//src
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);
				srvIndex += (int)ESRV_OneEntry::eNum;
			}
		);

		return srvIndex;
	}

	void RayTracingEngine::Dispatch(ID3D12GraphicsCommandList4* commandList)
	{
		if (!m_isCommit) {
			return;//�R�~�b�g���ĂȂ�
		}

		//�萔�X�V
		ReyTracingCBStructure cam;
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
		if (!m_world.GetIsUpdated()) {
			if (m_isCommit) {
				m_world.UpdateTLAS(commandList);//TLAS�̂ݍX�V
			}
			return;//�X�V�Ȃ�
		}

		//AS�̍\�z
		//TODO �X�V����@(���̂܂܂ł�����H)
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
		//TODO �X�V����@(���̂܂܂ł�����H)
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
		m_cbStructure.pos = GetMainCamera()->GetPos();
		m_cbStructure.mRot = GetMainCamera()->GetRotMatrix();
		m_cbStructure.aspect = GetMainCamera()->GetAspect();
		m_cbStructure.fNear = GetMainCamera()->GetNear();
		m_cbStructure.fFar = GetMainCamera()->GetFar();
		m_rayGenerationCB.Init(sizeof(ReyTracingCBStructure), &m_cbStructure);
	}
}