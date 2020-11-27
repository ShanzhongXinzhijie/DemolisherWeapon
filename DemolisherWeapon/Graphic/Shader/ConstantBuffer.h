#pragma once
#include"Graphic/Shader/ShaderUtil.h"

namespace DemolisherWeapon {

	class IConstantBufferInner {
	public:
		virtual ~IConstantBufferInner() {

		}

		/// <summary>
		/// �������B
		/// </summary>
		/// <param name="size">�萔�o�b�t�@�̃T�C�Y�B</param>
		/// <param name="srcData">�\�[�X�f�[�^�Bnull���w�肷�邱�Ƃ��\�B</param>
		virtual void Create(int size, void* srcData = nullptr) = 0;

		/// <summary>
		/// �X�V
		/// </summary>
		/// <param name="srcData"></param>
		virtual void Update(const void* srcData) = 0;
	};

	class ConstantBufferDx11 : public IConstantBufferInner {
	public:
		/// <summary>
		/// �������B
		/// </summary>
		/// <param name="size">�萔�o�b�t�@�̃T�C�Y�B</param>
		/// <param name="srcData">�\�[�X�f�[�^�Bnull���w�肷�邱�Ƃ��\�B</param>
		void Create(int size, void* srcData = nullptr)override {
			//�쐬
			ShaderUtil::CreateConstantBuffer(size, m_cb.ReleaseAndGetAddressOf());
			//�f�[�^�R�s�[
			if (srcData) {
				Update(srcData);
			}
		}

		/// <summary>
		/// �X�V
		/// </summary>
		/// <param name="srcData"></param>
		void Update(const void* srcData)override {
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_cb.Get(), 0, nullptr, srcData, 0, 0);
		}

		/// <summary>
		/// �o�b�t�@�̎擾
		/// </summary>
		/// <returns></returns>
		ID3D11Buffer* GetBuffer() {
			return m_cb.Get();
		}
		ID3D11Buffer** GetAddressOfBuffer() {
			return m_cb.GetAddressOf();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_cb;
	};

	/// <summary>
	/// �萔�o�b�t�@
	/// �o�b�N�o�b�t�@�̐���������...
	/// </summary>
	class ConstantBufferDx12 : public IConstantBufferInner {
	public:
		ConstantBufferDx12() = default;

		/// <summary>
		/// �f�X�g���N�^�B
		/// </summary>
		~ConstantBufferDx12() {
			Release();
		}

		/// <summary>
		/// �������B
		/// </summary>
		/// <param name="size">�萔�o�b�t�@�̃T�C�Y�B</param>
		/// <param name="srcData">�\�[�X�f�[�^�Bnull���w�肷�邱�Ƃ��\�B</param>
		void Create(int size, void* srcData = nullptr)override {
			Release();

			m_size = size;
			//�萔�o�b�t�@��256�o�C�g�A���C�����g���v�������̂ŁA256�̔{���ɐ؂�グ��B
			m_allocSize = (size + 256) & 0xFFFFFF00;
			//�萔�o�b�t�@�̍쐬�B
			auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_allocSize);
			GetGraphicsEngine().GetD3D12Device()->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
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
			//CBV�쐬
			CreateConstantBufferView();
		}

		/// <summary>
		/// �萔�o�b�t�@�̒��g���X�V�B
		/// </summary>
		/// <param name="data"></param>
		void Update(const void* data)override {
			memcpy(m_constBufferCPU, data, m_size);
		}		

		/// <summary>
		/// CPU�f�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle()const {
			return m_cpuHandle;
		}
		/// <summary>
		/// GPU�f�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle()const {
			return m_gpuHandle;
		}

	private:
		/// <summary>
		/// ConstantBufferView�̍쐬
		/// </summary>
		void CreateConstantBufferView() {
			auto [gpu, cpu] = GetGraphicsEngine().GetDX12().CreateConstantBufferView(m_constantBuffer.Get(), m_allocSize);
			m_cpuHandle = cpu;
			m_gpuHandle = gpu;
		}

		void Release() {
			if (m_constantBuffer) {
				CD3DX12_RANGE readRange(0, 0);
				m_constantBuffer->Unmap(0, &readRange);
			}
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_constantBuffer;	//�萔�o�b�t�@�B
		void* m_constBufferCPU = nullptr;							//CPU������A�N�Z�X�ł��邷��萔�o�b�t�@�̃A�h���X�B
		int m_size = 0;												//�萔�o�b�t�@�̃T�C�Y�B
		int m_allocSize = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

	/// <summary>
	/// �萔�o�b�t�@
	/// </summary>
	template<class T>
	class ConstantBuffer {
	public:
		/// <summary>
		/// �������B
		/// </summary>
		/// <param name="size">�萔�o�b�t�@�̃T�C�Y�B</param>
		/// <param name="srcData">�\�[�X�f�[�^�Bnull���w�肷�邱�Ƃ��\�B</param>
		void Init(int size, T* srcData = nullptr) {
			if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
				m_innerClass = std::make_unique<ConstantBufferDx11>();
			}
			if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
				m_innerClass = std::make_unique<ConstantBufferDx12>();
			}
			m_innerClass->Create(size, srcData);
		}

		/// <summary>
		/// �萔�o�b�t�@�̒��g���X�V�B
		/// </summary>
		/// <param name="data"></param>
		void Update(const T* data) {
			m_innerClass->Update(data);
		}

		/// <summary>
		/// �o�b�t�@�̎擾(DX11)
		/// </summary>
		/// <returns></returns>
		ID3D11Buffer* GetBuffer()const {
			ConstantBufferDx11* cb11 = dynamic_cast<ConstantBufferDx11*>(m_innerClass.get());
			if (cb11 == nullptr) { return nullptr; }

			return cb11->GetBuffer();
		}
		ID3D11Buffer** GetAddressOfBuffer()const {
			ConstantBufferDx11* cb11 = dynamic_cast<ConstantBufferDx11*>(m_innerClass.get());
			if (cb11 == nullptr) { return nullptr; }

			return cb11->GetAddressOfBuffer();
		}

		/// <summary>
		/// CPU�f�X�N���v�^�n���h���擾(DX12)
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle()const {
			ConstantBufferDx12* cb12 = dynamic_cast<ConstantBufferDx12*>(m_innerClass.get());
			if (cb12 == nullptr) { return D3D12_CPU_DESCRIPTOR_HANDLE(); }
			
			return cb12->GetCPUDescriptorHandle();
		}

	private:
		std::unique_ptr<IConstantBufferInner> m_innerClass;		
	};
}
