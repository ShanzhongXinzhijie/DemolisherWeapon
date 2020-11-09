#pragma once

namespace DemolisherWeapon {

	class IStructuredBufferInner {
	public:
		virtual void Init(int num, int stride) = 0;
		virtual void UpdateSubresource(void* data) = 0;
	};

	class StructuredBufferInnerDX11 : public IStructuredBufferInner {
	public:
		void Init(int num, int stride)override {
			//StructuredBuffer�̊m��
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.ByteWidth = static_cast<UINT>(stride * num);
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			desc.StructureByteStride = stride;
			GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, m_SB.ReleaseAndGetAddressOf());

			//ShaderResourceView�̊m��
			D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
			ZeroMemory(&descSRV, sizeof(descSRV));
			descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descSRV.BufferEx.FirstElement = 0;
			descSRV.Format = DXGI_FORMAT_UNKNOWN;
			descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
			GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_SB.Get(), &descSRV, m_SRV.ReleaseAndGetAddressOf());
		}

		void UpdateSubresource(void* data)override {
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
				m_SB.Get(), 0, NULL, data, 0, 0
			);
		}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetSRV() {
			return m_SRV;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_SB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;
	};

	class StructuredBufferInnerDX12 : public IStructuredBufferInner {
	public:
		void Init(int num, int stride)override {
			DW_ERRORBOX(true,"StructuredBufferInnerDX12::Init ���Ή��ł�")
		}

		void Init(const VertexBufferDX12& vb, bool isUpdateByCPU) {
			m_sizeOfElement = vb.GetStrideInBytes();
			m_numElement = vb.GetSizeInBytes() / m_sizeOfElement;
			if (isUpdateByCPU) {
				DW_ERRORBOX(true, "StructuredBufferInnerDX12::Init isUpdateByCPU�͖��Ή��ł�")
			}
			else {
				for (auto& gpuBuffer : m_buffersOnGPU) {
					gpuBuffer = vb.GetResource();
					gpuBuffer->AddRef();
				}
				//CPU����͕ύX�ł��Ȃ��̂Ń}�b�v���Ȃ��B
				for (auto& cpuBuffer : m_buffersOnCPU) {
					cpuBuffer = nullptr;
				}
			}
			CreateShaderResourceView();
		}
		void Init(const IndexBufferDX12& ib, bool isUpdateByCPU) {
			m_sizeOfElement = ib.GetStrideInBytes();
			m_numElement = ib.GetSizeInBytes() / m_sizeOfElement;
			if (isUpdateByCPU) {
				DW_ERRORBOX(true, "StructuredBufferInnerDX12::Init isUpdateByCPU�͖��Ή��ł�")
			}
			else {
				for (auto& gpuBuffer : m_buffersOnGPU) {
					gpuBuffer = ib.GetResource();
					gpuBuffer->AddRef();
				}
				//CPU����͕ύX�ł��Ȃ��̂Ń}�b�v���Ȃ��B
				for (auto& cpuBuffer : m_buffersOnCPU) {
					cpuBuffer = nullptr;
				}
			}
			CreateShaderResourceView();
		}

		void UpdateSubresource(void* data)override {
			DW_ERRORBOX(true, "StructuredBufferInnerDX12::UpdateSubresource ���Ή��ł�")
		}		

		/// <summary>
		/// CPU�f�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() {
			return m_cpuHandle;
		}
		/// <summary>
		/// GPU�f�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() {
			return m_gpuHandle;
		}

	private:
		/// <summary>
		/// SRV�쐬
		/// </summary>
		void CreateShaderResourceView()
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<UINT>(m_numElement);
			srvDesc.Buffer.StructureByteStride = m_sizeOfElement;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			auto [gpu, cpu] = GetGraphicsEngine().GetDX12().CreateSRV(m_buffersOnGPU[0].Get(), &srvDesc);
			m_cpuHandle = cpu;
			m_gpuHandle = gpu;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffersOnGPU[2] = { nullptr };
		void* m_buffersOnCPU[2] = { nullptr };	//CPU������A�N�Z�X�ł��邷��X�g���N�`���o�b�t�@�̃A�h���X�B
		int m_numElement = 0;					//�v�f���B
		int m_sizeOfElement = 0;				//�G�������g�̃T�C�Y�B

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

	template<class T>
	class StructuredBuffer
	{
	public:
		StructuredBuffer() = default;
		StructuredBuffer(int num) { Init(num); }

		void Init(int num = 1) {
			//�z��̊m��
			m_t = std::make_unique<T[]>(num);
			int stride = sizeof(T);

			if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
				m_innerIns = std::make_unique<StructuredBufferInnerDX11>();
			}
			m_innerIns->Init(num, stride);
		}

		//�f�[�^���擾
		const std::unique_ptr<T[]>& GetData()const { return m_t; }
		std::unique_ptr<T[]>& GetData() { return m_t; }

		//StructuredBuffer���X�V
		void UpdateSubresource() {
			m_innerIns->UpdateSubresource(m_t.get());
		}

		//SRV�̎擾(DX11)
		ID3D11ShaderResourceView* GetSRV()const {
			StructuredBufferInnerDX11* sb11 = dynamic_cast<StructuredBufferInnerDX11*>(m_innerIns.get());
			if (sb11 == nullptr) { return nullptr; }
			return sb11->GetSRV().Get();
		}
		ID3D11ShaderResourceView*const* GetAddressOfSRV()const {
			StructuredBufferInnerDX11* sb11 = dynamic_cast<StructuredBufferInnerDX11*>(m_innerIns.get());
			if (sb11 == nullptr) { return nullptr; }
			return sb11->GetSRV().GetAddressOf();
		}

	private:
		std::unique_ptr<T[]>					m_t;
		std::unique_ptr<IStructuredBufferInner> m_innerIns;
	};

}