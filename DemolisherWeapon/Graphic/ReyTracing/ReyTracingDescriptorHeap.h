#pragma once

namespace DemolisherWeapon {
	class ReyTracingWorld;

	/// <summary>
	/// ���C�g���̃f�B�X�N���v�^�q�[�v
	/// </summary>
	class ReyTracingDescriptorHeap {
	public:
		/// <summary>
		/// ������
		/// </summary>
		void Init(
			ReyTracingWorld& world,
			ConstantBuffer<ReyTracingCBStructure>& cb,
			D3D12_CPU_DESCRIPTOR_HANDLE uavHandle
		);

		/// <summary>
		/// �X�V
		/// </summary>
		int Update(ReyTracingWorld& world);

		//�f�B�X�N���v�^�q�[�v�擾
		ID3D12DescriptorHeap* GetSRVHeap()const {
			return m_srvsDescriptorHeap.Get();
		}
		ID3D12DescriptorHeap* GetSamplerHeap()const {
			return m_samplerDescriptorHeap.Get();
		}

		/// <summary>
		/// SRV�f�X�N���v�^�̃T�C�Y
		/// </summary>
		/// <returns></returns>
		UINT GetSrvsDescriptorSize()const {
			return m_srvsDescriptorSize;
		}

		/// <summary>
		/// UAV�f�X�N���v�^�̊J�n�ʒu
		/// </summary>
		/// <returns></returns>
		int GetOffsetUAVDescriptorFromTableStart()const {
			return m_uavStartNum;
		}
		/// <summary>
		/// SRV�f�X�N���v�^�̊J�n�ʒu
		/// </summary>
		/// <returns></returns>
		int GetOffsetSRVDescriptorFromTableStart()const {
			return m_srvStartNum;
		}
		/// <summary>
		/// CBV�f�X�N���v�^�̊J�n�ʒu
		/// </summary>
		/// <returns></returns>
		int GetOffsetCBVDescriptorFromTableStart()const {
			return m_cbvStartNum;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvsDescriptorHeap;
		UINT m_srvsDescriptorSize = 0;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerDescriptorHeap;
		UINT m_samplerDescriptorSize = 0;

		int m_uavStartNum = 0;
		int m_srvStartNum = 0;
		int m_cbvStartNum = 0;
	};

}