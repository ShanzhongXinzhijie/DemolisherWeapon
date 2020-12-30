#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// ���C�g���p�̃p�C�v���C���X�e�[�g�I�u�W�F�N�g�B
	/// </summary>
	class ReyTracingPSO {
	private:
		/// <summary>
		/// ���[�g�V�O�l�`����`�B
		/// </summary>
		struct RootSignatureDesc
		{
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			std::vector<D3D12_DESCRIPTOR_RANGE> range;
			std::vector<D3D12_ROOT_PARAMETER> rootParams;
		};

	public:
		/// <summary>
		/// �p�C�v���C���X�e�[�g�I�u�W�F�N�g���������B
		/// </summary>
		void Init(const ReyTracingDescriptorHeap* DH);

		void QueryInterface(Microsoft::WRL::ComPtr<ID3D12StateObjectProperties>& props) const
		{
			m_pipelineState.As(&props);
			//m_pipelineState->QueryInterface(&props);
		}

		/// <summary>
		/// �O���[�o�����[�g�V�O�l�`�����擾�B
		/// </summary>
		/// <returns></returns>
		ID3D12RootSignature* GetGlobalRootSignature()
		{
			return m_emptyRootSignature.Get();
		}

		ID3D12StateObject* Get()
		{
			return m_pipelineState.Get();
		}

	private:
		RootSignatureDesc CreateRayGenRootSignatureDesc();
		RootSignatureDesc CreatePBRMatterialHitRootSignatureDesc();

	private:
		const ReyTracingDescriptorHeap* m_descriptorHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12StateObject> m_pipelineState;					//�p�C�v���C���X�e�[�g
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_emptyRootSignature;
	};

}