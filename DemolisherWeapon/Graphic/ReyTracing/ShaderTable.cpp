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
		auto hitGroup_pbrCameraRaySrvHeapStride = ds_size_cbv_srv_uav * (int)ESRV_OneEntry::eNum;

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

}