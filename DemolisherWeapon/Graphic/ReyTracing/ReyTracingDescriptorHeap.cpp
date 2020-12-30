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
		//TODO �����ɂ���?
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

}