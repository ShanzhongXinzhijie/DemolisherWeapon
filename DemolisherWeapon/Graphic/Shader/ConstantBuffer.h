#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// �萔�o�b�t�@
	/// �o�b�N�o�b�t�@�̐���������...
	/// </summary>
	template<class T>
	class ConstantBufferDx12 {
	public:
		ConstantBufferDx12() = default;

		/// <summary>
		/// �f�X�g���N�^�B
		/// </summary>
		~ConstantBufferDx12() {
			CD3DX12_RANGE readRange(0, 0);
			m_constantBuffer->Unmap(0, &readRange);
		}

		/// <summary>
		/// �������B
		/// </summary>
		/// <param name="size">�萔�o�b�t�@�̃T�C�Y�B</param>
		/// <param name="srcData">�\�[�X�f�[�^�Bnull���w�肷�邱�Ƃ��\�B</param>
		void Init(int size, T* srcData = nullptr) {
			m_size = size;
			//�萔�o�b�t�@��256�o�C�g�A���C�����g���v�������̂ŁA256�̔{���ɐ؂�グ��B
			m_allocSize = (size + 256) & 0xFFFFFF00;
			//�萔�o�b�t�@�̍쐬�B
			GetGraphicsEngine().GetD3D12Device()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_allocSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_constantBuffer)
			);
			//�萔�o�b�t�@��CPU����A�N�Z�X�\�ȉ��z�A�h���X��ԂɃ}�b�s���O����B
			//�}�b�v�A�A���}�b�v�̃I�[�o�[�w�b�h���y�����邽�߂ɂ͂��̃C���X�^���X�������Ă���Ԃ͍s��Ȃ��B
			{
				CD3DX12_RANGE readRange(0, 0);
				m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_constBufferCPU));
			}
			if (srcData != nullptr) {
				memcpy(m_constBufferCPU, srcData, m_size);
			}
		}

		/// <summary>
		/// �萔�o�b�t�@�̒��g���X�V�B
		/// </summary>
		/// <param name="data"></param>
		void Update(T* data)  {
			memcpy(m_constBufferCPU, data, m_size);
		}

		/// <summary>
		/// ConstantBufferView�̍쐬
		/// </summary>
		/// <param name="descriptorHandle">�f�B�X�N���v�^�̃n���h��</param>
		void CreateConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle){
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
			desc.SizeInBytes = m_allocSize;
			GetGraphicsEngine().GetD3D12Device()->CreateConstantBufferView(&desc, descriptorHandle);
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_constantBuffer;	//�萔�o�b�t�@�B
		T* m_constBufferCPU = nullptr;								//CPU������A�N�Z�X�ł��邷��萔�o�b�t�@�̃A�h���X�B
		int m_size = 0;												//�萔�o�b�t�@�̃T�C�Y�B
		int m_allocSize = 0;
	};

}
