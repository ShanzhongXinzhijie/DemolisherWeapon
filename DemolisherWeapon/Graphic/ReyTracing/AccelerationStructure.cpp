#include "DWstdafx.h"
#include "AccelerationStructure.h"

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

		constexpr int MAX_ONE_BARRIERS = 100;
		D3D12_RESOURCE_BARRIER maximumBarriers[MAX_ONE_BARRIERS];
	}

	void BLASBuffer::Init(
		ID3D12GraphicsCommandList4* commandList,
		const std::vector<std::unique_ptr<ReyTracingGeometoryData>>& geometories
	)
	{
		auto d3dDevice = GetGraphicsEngine().GetD3D12Device();

		int barriorNum = 0;

		for (auto& geometory : geometories) {
			//すでに構築済み
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
				L"BLASpScratch"
			);
			CreateBuffer(
				d3dDevice,
				info.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				kDefaultHeapProps,
				asbuffer.pResult,
				L"BLASpResult"
			);

			// Create the bottom-level AS
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
			asDesc.Inputs = inputs;
			asDesc.DestAccelerationStructureData = asbuffer.pResult->GetGPUVirtualAddress();
			asDesc.ScratchAccelerationStructureData = asbuffer.pScratch->GetGPUVirtualAddress();
			commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

			//レイトレーシングアクセラレーション構造のビルド完了待ちのバリアを入れる。
			maximumBarriers[barriorNum].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			maximumBarriers[barriorNum].UAV.pResource = asbuffer.pResult.Get();
			barriorNum++;
			//最大数を超えたらここまでのバリアを貼る
			if (barriorNum >= MAX_ONE_BARRIERS) {
				DW_WARNING_MESSAGE(true, "BLASBuffer::Init() barriorNumがMAX_ONE_BARRIERSを超えました\n")
					commandList->ResourceBarrier(barriorNum, maximumBarriers);
				barriorNum = 0;
			}

			m_bottomLevelASBuffers.push_back(std::move(asbuffer));
			geometory->m_pResultBLAS = m_bottomLevelASBuffers.back().pResult.Get();
		}

		if (barriorNum > 0) {
			commandList->ResourceBarrier(barriorNum, maximumBarriers);
		}
	}

	void TLASBuffer::Init(
		ID3D12GraphicsCommandList4* commandList,
		const std::list<std::unique_ptr<ReyTracingInstanceData>>& instances,
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

		if (update) {
			//更新
			D3D12_RESOURCE_BARRIER uavBarrier = {};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = m_topLevelASBuffers.pResult.Get();
			commandList->ResourceBarrier(1, &uavBarrier);
		}
		else {
			//新規
			CreateBuffer(
				d3dDevice,
				info.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				kDefaultHeapProps,
				m_topLevelASBuffers.pScratch,
				L"TLASpScratch"
			);
			CreateBuffer(
				d3dDevice,
				info.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
				kDefaultHeapProps,
				m_topLevelASBuffers.pResult,
				L"TLASpResult"
			);
			CreateBuffer(
				d3dDevice,
				sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numInstance,
				D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
				kUploadHeapProps,
				m_topLevelASBuffers.pInstanceDesc,
				L"TLASpInstanceDesc"
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

		{
			int i = 0;
			for (auto& ins : instances) {
				instanceDescs[i].InstanceID = i;
				instanceDescs[i].InstanceContributionToHitGroupIndex = (int)eHitGroup_Num * i + eHitGroup_PBRCameraRay;//使用するヒットグループ
				instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
				instanceDescs[i].AccelerationStructure = ins->m_geometory->m_pResultBLAS->GetGPUVirtualAddress();
				instanceDescs[i].InstanceMask = 0xFF;//ゼロだとレイに当たらない //これで非表示とかやる?

				//トランスフォーム行列設定
				CMatrix mTrans = *(ins->m_worldMatrix);
				mTrans.Transpose();
				memcpy(instanceDescs[i].Transform, &mTrans, sizeof(instanceDescs[i].Transform));

				i++;
			}
		}

		m_topLevelASBuffers.pInstanceDesc->Unmap(0, nullptr);

		//TopLevelASを作成。
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

		//レイトレーシングアクセラレーション構造のビルド完了待ちのバリアを入れる。
		D3D12_RESOURCE_BARRIER uavBarrier = {};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = m_topLevelASBuffers.pResult.Get();
		commandList->ResourceBarrier(1, &uavBarrier);
	}
	void TLASBuffer::CreateShaderResourceView()
	{
		//TLASをディスクリプタヒープに登録。
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();

		auto [gpu, cpu] = GetGraphicsEngine().GetDX12().CreateSRV(nullptr, &srvDesc);
		m_GPUdescriptorHandle = gpu;
		m_CPUdescriptorHandle = cpu;
	}

}