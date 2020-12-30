#include "DWstdafx.h"
#include "ReyTracingDescriptorHeap.h"

namespace DemolisherWeapon {

	namespace {
		constexpr int SRVS_DESC_NUM = 1000000;
		constexpr int SAMPLER_DESC_NUM = 3;
	}

	void ReyTracingDescriptorHeap::Init(ReyTracingWorld& world, ConstantBuffer<ReyTracingCBStructure>& cb, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle)
	{
		auto device = GetGraphicsEngine().GetD3D12Device();

		//CBV_SRV_UAV用のデスクリプタヒープ作成
		m_srvsDescriptorSize = GetGraphicsEngine().GetDX12().CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SRVS_DESC_NUM, true, m_srvsDescriptorHeap);
		//サンプラー用のデスクリプタヒープ作成
		m_samplerDescriptorSize = GetGraphicsEngine().GetDX12().CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SAMPLER_DESC_NUM, true, m_samplerDescriptorHeap);

		int srvIndex = 0;

		//CBV開始位置
		m_cbvStartNum = srvIndex;
		//定数		
		device->CopyDescriptorsSimple(
			1,
			m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),//dest
			cb.GetCPUDescriptorHandle(),//src
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);
		srvIndex++;

		//UAV開始位置
		m_uavStartNum = srvIndex;
		//UAVの順
		device->CopyDescriptorsSimple(
			1,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex, m_srvsDescriptorSize),//dest
			uavHandle,//src
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);
		srvIndex++;

		//SRV開始位置
		m_srvStartNum = srvIndex;
		//SRV
		srvIndex = Update(world);

		//サンプラ
		D3D12_SAMPLER_DESC samplerDesc = {};
		if (GetGraphicsEngine().GetIsPointFiltering()) {
			samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		}
		else {
			samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
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
		//作成
		device->CreateSampler(&samplerDesc, m_samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	int ReyTracingDescriptorHeap::Update(ReyTracingWorld& world) {
		auto device = GetGraphicsEngine().GetD3D12Device();
		int srvIndex = m_srvStartNum;

		//TLASのSRV作成
		world.GetTLASBuffer().CreateShaderResourceView();

		//SRV
		world.QueryInstances([&](ReyTracingInstanceData& instance)
		{
			//TLASの登録
		//TODO ここにいる?
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eTLAS, m_srvsDescriptorSize),//dest
				world.GetTLASBuffer().GetCPUDescriptorHandle(),//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);

			//アルベドマップをディスクリプタヒープに登録。
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eAlbedoMap, m_srvsDescriptorSize),//dest
				instance.m_material->GetAlbedoTextureData().CPUdescriptorHandle,//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);
			//法線マップをディスクリプタヒープに登録。
			//※今はダミー
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eNormalMap, m_srvsDescriptorSize),//dest
				GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
				//instance.m_material->GetUsingMaterialSetting().GetNormalTextureData().CPUdescriptorHandle,//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);
			//スペキュラマップをディスクリプタヒープに登録。
			//※今はダミー
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eSpecularMap, m_srvsDescriptorSize),//dest
				GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
				//instance.m_material->GetUsingMaterialSetting().GetLightingTextureData().CPUdescriptorHandle,//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);

			//リフレクションマップをディスクリプタヒープに登録。
			//※今はダミー
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eReflectionMap, m_srvsDescriptorSize),//dest
				GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);
			//屈折マップをディスクリプタヒープに登録。
			//※今はダミー
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eRefractionMap, m_srvsDescriptorSize),//dest
				GetGraphicsEngine().GetDX12().GetSrvsDescriptorDammyCPUHandle(),//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);

			//頂点バッファをディスクリプタヒープに登録。
			device->CopyDescriptorsSimple(
				1,
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex + (int)ESRV_OneEntry::eVertexBuffer, m_srvsDescriptorSize),//dest
				instance.m_geometory->m_vertexBufferRWSB.GetCPUDescriptorHandle(),//src
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);
			//インデックスバッファをディスクリプタヒープに登録。
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

}