#pragma once

namespace DemolisherWeapon {

	class IStructuredBufferInner {
	public:
		virtual ~IStructuredBufferInner() {

		}

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="num">�v�f�̐�</param>
		/// <param name="stride">�v�f�̃T�C�Y</param>
		virtual void Init(int num, int stride) = 0;
		/// <summary>
		/// �X�V
		/// </summary>
		/// <param name="data">�\�[�X�f�[�^</param>
		virtual void UpdateSubresource(void* data) = 0;
	};

	class StructuredBufferInnerDX11 : public IStructuredBufferInner {
	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="num">�v�f�̐�</param>
		/// <param name="stride">�v�f�̃T�C�Y</param>
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

		/// <summary>
		/// �X�V
		/// </summary>
		/// <param name="data">�\�[�X�f�[�^</param>
		void UpdateSubresource(void* data)override {
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
				m_SB.Get(), 0, NULL, data, 0, 0
			);
		}

		/// <summary>
		/// SRV�擾
		/// </summary>
		/// <returns></returns>
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetSRV() {
			return m_SRV;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_SB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;
	};

	class StructuredBufferInnerDX12 : public IStructuredBufferInner {
	public:
		~StructuredBufferInnerDX12() {
			Release();
		}

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="num">�v�f�̐�</param>
		/// <param name="stride">�v�f�̃T�C�Y</param>
		void Init(int num, int stride) override{
			Release();

			m_sizeOfElement = stride;
			m_numElement = num;

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(m_sizeOfElement * m_numElement);
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			D3D12_HEAP_PROPERTIES prop{};
			prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			prop.CreationNodeMask = 1;
			prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			prop.Type = D3D12_HEAP_TYPE_CUSTOM;
			prop.VisibleNodeMask = 1;

			int bufferNo = 0;
			for (auto& buffer : m_buffersOnGPU) {
				//�o�b�t�@�쐬
				GetGraphicsEngine().GetD3D12Device()->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					nullptr,
					IID_PPV_ARGS(&buffer)
				);

				//�\�����o�b�t�@��CPU����A�N�Z�X�\�ȉ��z�A�h���X��ԂɃ}�b�s���O����B
				//�}�b�v�A�A���}�b�v�̃I�[�o�[�w�b�h���y�����邽�߂ɂ͂��̃C���X�^���X�������Ă���Ԃ͍s��Ȃ��B
				CD3DX12_RANGE readRange(0, 0);
				buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_buffersOnCPU[bufferNo]));
				
				//SRV�쐬
				CreateShaderResourceView(bufferNo);

				bufferNo++;
			}
		}

		/// <summary>
		/// �X�V
		/// </summary>
		/// <param name="data">�\�[�X�f�[�^</param>
		void UpdateSubresource(void* data)override {
			if (m_buffersOnCPU[GetGraphicsEngine().GetDX12().GetCurrentBackBufferIndex()] == nullptr) {
				DW_WARNING_BOX(true, "StructuredBufferInnerDX12::UpdateSubresource\n�X�V�s��")
			}
			//TODO ���t���[������Ă��ׂĂ̂�X�V���Ȃ���...
			////�擾�n�̂�ł�������s?
			//�����_�[�^�[�Q�b�g�̃C���f�b�N�X�X�V��
			//�����_�[�I�Ȃ̂Ŏ��s?
			memcpy(m_buffersOnCPU[GetGraphicsEngine().GetDX12().GetCurrentBackBufferIndex()], data, m_sizeOfElement * m_numElement);
		}

		/// <summary>
		/// ���_�o�b�t�@���珉����
		/// </summary>
		/// <param name="vb"></param>
		/// <param name="isUpdateByCPU"></param>
		void Init(const VertexBufferDX12& vb, bool isUpdateByCPU) {
			Release();

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
			//SRV�쐬
			int i = 0;
			for (auto& gpuBuffer : m_buffersOnGPU) {
				CreateShaderResourceView(i);
				i++;
			}
		}
		/// <summary>
		/// �C���f�b�N�X�o�b�t�@���珉����
		/// </summary>
		/// <param name="ib"></param>
		/// <param name="isUpdateByCPU"></param>
		void Init(const IndexBufferDX12& ib, bool isUpdateByCPU) {
			Release();

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
			//SRV�쐬
			int i = 0;
			for (auto& gpuBuffer : m_buffersOnGPU) {
				CreateShaderResourceView(i);
				i++;
			}
		}

		/// <summary>
		/// CPU�f�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle()const {
			return m_cpuHandle[GetGraphicsEngine().GetDX12().GetCurrentBackBufferIndex()];
		}
		/// <summary>
		/// GPU�f�X�N���v�^�n���h���擾
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle()const {
			return m_gpuHandle[GetGraphicsEngine().GetDX12().GetCurrentBackBufferIndex()];
		}

	private:
		/// <summary>
		/// SRV�쐬
		/// </summary>
		void CreateShaderResourceView(int index)
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

			auto [gpu, cpu] = GetGraphicsEngine().GetDX12().CreateSRV(m_buffersOnGPU[index].Get(), &srvDesc);
			m_cpuHandle[index] = cpu;
			m_gpuHandle[index] = gpu;
		}

		void Release() {
			//�A���}�b�v
			CD3DX12_RANGE readRange(0, 0);
			for (auto& buffer : m_buffersOnGPU) {
				if (buffer) {
					buffer->Unmap(0, &readRange);
					buffer->Release();
				}
			}
			for (auto& buffer : m_buffersOnCPU) {
				buffer = nullptr;
			}
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffersOnGPU[DX12Test::FRAME_COUNT];
		void* m_buffersOnCPU[DX12Test::FRAME_COUNT] = { nullptr };//CPU������A�N�Z�X�ł��邷��X�g���N�`���o�b�t�@�̃A�h���X�B
		int m_numElement = 0;	//�v�f���B
		int m_sizeOfElement = 0;//�G�������g�̃T�C�Y�B

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle[DX12Test::FRAME_COUNT];
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle[DX12Test::FRAME_COUNT];
	};

	template<class T>
	class StructuredBuffer
	{
	public:
		//�R���X�g���N�^
		StructuredBuffer() = default;
		StructuredBuffer(int num) { Init(num); }

		//������
		void Init(int num = 1) {
			//�z��̊m��
			m_t = std::make_unique<T[]>(num);
			int stride = sizeof(T);

			if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
				m_innerIns = std::make_unique<StructuredBufferInnerDX11>();
			}
			if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
				m_innerIns = std::make_unique<StructuredBufferInnerDX12>();
			}
			m_innerIns->Init(num, stride);
		}

		//�J��
		void Release() {
			m_t.reset();
			m_innerIns.reset();
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