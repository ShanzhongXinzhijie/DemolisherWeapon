#include "DWstdafx.h"
#include "ShaderTable.h"

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

	void ShaderTable::CountupNumGeyGenAndMissAndHitShader()
	{
		//各シェーダーの数を調査。
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
		//シェーダーテーブルに登録されるデータの１要素分のサイズを計算。
		//シェーダー識別子。
		m_shaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		//ディスクリプタテーブルのアドレス。
		m_shaderTableEntrySize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE) * eHitShaderDescriptorTable_Num; // The hit shader
		//アライメントをそろえる。
		m_shaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, m_shaderTableEntrySize);
	}
	void ShaderTable::Init(
		const ReyTracingWorld& world,
		const ReyTracingPSO& pso,
		const ReyTracingDescriptorHeap& descriptorHeaps
	)
	{
		//各シェーダーの数をカウントする。
		CountupNumGeyGenAndMissAndHitShader();

		//シェーダーテーブルに登録されるデータの１要素分のサイズを計算。
		CalcShaderTableEntrySize();

		//シェーダーテーブルのサイズを計算。
		int shaderTableSize = m_shaderTableEntrySize * (m_numRayGenShader + m_numMissShader + (m_numHitShader * world.GetNumInstance()));

		//シェーダーテーブル用のバッファを作成。
		CreateBuffer(
			GetGraphicsEngine().GetD3D12Device(),
			shaderTableSize,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			kUploadHeapProps,
			m_shaderTable,
			L"ShaderTableBuffer"
		);

		//バッファをシステムメモリにマップする。
		uint8_t* pData;
		m_shaderTable->Map(0, nullptr, (void**)&pData);

		Microsoft::WRL::ComPtr< ID3D12StateObjectProperties > pRtsoProps;
		pso.QueryInterface(pRtsoProps);

		uint8_t* pCurret = pData;

		auto ds_size_cbv_srv_uav = descriptorHeaps.GetSrvsDescriptorSize();
		auto hitGroup_pbrCameraRaySrvHeapStride = ds_size_cbv_srv_uav * (int)ESRV_OneEntry::eNum;

		const auto& srvUavCbvDescriptorHeapStart = descriptorHeaps.GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
		const auto& samplerDescriptorHeapStart = descriptorHeaps.GetSamplerHeap()->GetGPUDescriptorHandleForHeapStart();

		//ローカルルートシグネチャ"eLocalRootSignature_PBRMaterialHit"のSRV開始位置
		uint64_t hitGroup_pbrCameraRaySrvHeapStart = srvUavCbvDescriptorHeapStart.ptr + ds_size_cbv_srv_uav * descriptorHeaps.GetOffsetSRVDescriptorFromTableStart();

		//シェーダーテーブルにシェーダーを登録する関数。
		auto RegistShaderTblFunc = [&](const ShaderData& shaderData, EShaderCategory registCategory, ReyTracingInstanceData* instance)
		{
			if (shaderData.category == registCategory) {
				//まずシェーダーIDを設定する。
				void* pShaderId = nullptr;
				if (registCategory == eShaderCategory_ClosestHit) {
					pShaderId = pRtsoProps->GetShaderIdentifier(hitGroups[shaderData.hitgroup].name);
				}
				else {
					pShaderId = pRtsoProps->GetShaderIdentifier(shaderData.entryPointName);
				}
				memcpy(pCurret, pShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

				//アドレス進める
				uint8_t* pDst = pCurret + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

				//各ディスクリプタヒープの始まるアドレスをセット?
				if (shaderData.useLocalRootSignature == eLocalRootSignature_Raygen) {
					//デフォルトのルートシグネチャを使用する場合、シェーダーIDの後ろにディスクリプタヒープへのアドレスを設定する。
					*(uint64_t*)(pDst) = srvUavCbvDescriptorHeapStart.ptr;
				}
				if (shaderData.useLocalRootSignature == eLocalRootSignature_PBRMaterialHit) {
					//SRV_CBVのディスクリプタヒープ				
					*(uint64_t*)(pDst) = hitGroup_pbrCameraRaySrvHeapStart;
					//次
					pDst += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
					//サンプラーのディスクリプタヒープ
					*(uint64_t*)(pDst) = samplerDescriptorHeapStart.ptr;
				}

				//次。
				pCurret += m_shaderTableEntrySize;
			}
		};
		// レイジェネレーションシェーダーをテーブルに登録していく。
		for (auto& shader : shaderDatas) {
			RegistShaderTblFunc(shader, eShaderCategory_RayGenerator, nullptr);
		}
		// 続いてミスシェーダー。
		for (auto& shader : shaderDatas) {
			RegistShaderTblFunc(shader, eShaderCategory_Miss, nullptr);
		}
		//最後にヒットシェーダー。ヒットシェーダーはヒットシェーダーの数　×　インスタンスの数だけ登録する。
		world.QueryInstances([&](ReyTracingInstanceData& instance) {
			for (auto& shader : shaderDatas) {
				RegistShaderTblFunc(shader, eShaderCategory_ClosestHit, &instance);
			};
			hitGroup_pbrCameraRaySrvHeapStart += hitGroup_pbrCameraRaySrvHeapStride;	//次
		});

		//Unmap
		m_shaderTable->Unmap(0, nullptr);
	}

}