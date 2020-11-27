#pragma once
#include"Graphic/Shader/ShaderUtil.h"

namespace DemolisherWeapon {

	class IConstantBufferInner {
	public:
		virtual ~IConstantBufferInner() {

		}

		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="size">定数バッファのサイズ。</param>
		/// <param name="srcData">ソースデータ。nullを指定することも可能。</param>
		virtual void Create(int size, void* srcData = nullptr) = 0;

		/// <summary>
		/// 更新
		/// </summary>
		/// <param name="srcData"></param>
		virtual void Update(const void* srcData) = 0;
	};

	class ConstantBufferDx11 : public IConstantBufferInner {
	public:
		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="size">定数バッファのサイズ。</param>
		/// <param name="srcData">ソースデータ。nullを指定することも可能。</param>
		void Create(int size, void* srcData = nullptr)override {
			//作成
			ShaderUtil::CreateConstantBuffer(size, m_cb.ReleaseAndGetAddressOf());
			//データコピー
			if (srcData) {
				Update(srcData);
			}
		}

		/// <summary>
		/// 更新
		/// </summary>
		/// <param name="srcData"></param>
		void Update(const void* srcData)override {
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_cb.Get(), 0, nullptr, srcData, 0, 0);
		}

		/// <summary>
		/// バッファの取得
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
	/// 定数バッファ
	/// バックバッファの数だけいる...
	/// </summary>
	class ConstantBufferDx12 : public IConstantBufferInner {
	public:
		ConstantBufferDx12() = default;

		/// <summary>
		/// デストラクタ。
		/// </summary>
		~ConstantBufferDx12() {
			Release();
		}

		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="size">定数バッファのサイズ。</param>
		/// <param name="srcData">ソースデータ。nullを指定することも可能。</param>
		void Create(int size, void* srcData = nullptr)override {
			Release();

			m_size = size;
			//定数バッファは256バイトアライメントが要求されるので、256の倍数に切り上げる。
			m_allocSize = (size + 256) & 0xFFFFFF00;
			//定数バッファの作成。
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
			//定数バッファをCPUからアクセス可能な仮想アドレス空間にマッピングする。
			//マップ、アンマップのオーバーヘッドを軽減するためにはこのインスタンスが生きている間は行わない。
			{
				CD3DX12_RANGE readRange(0, 0);
				m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_constBufferCPU));
			}
			if (srcData != nullptr) {
				memcpy(m_constBufferCPU, srcData, m_size);
			}
			//CBV作成
			CreateConstantBufferView();
		}

		/// <summary>
		/// 定数バッファの中身を更新。
		/// </summary>
		/// <param name="data"></param>
		void Update(const void* data)override {
			memcpy(m_constBufferCPU, data, m_size);
		}		

		/// <summary>
		/// CPUデスクリプタハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle()const {
			return m_cpuHandle;
		}
		/// <summary>
		/// GPUデスクリプタハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle()const {
			return m_gpuHandle;
		}

	private:
		/// <summary>
		/// ConstantBufferViewの作成
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
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_constantBuffer;	//定数バッファ。
		void* m_constBufferCPU = nullptr;							//CPU側からアクセスできるする定数バッファのアドレス。
		int m_size = 0;												//定数バッファのサイズ。
		int m_allocSize = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

	/// <summary>
	/// 定数バッファ
	/// </summary>
	template<class T>
	class ConstantBuffer {
	public:
		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="size">定数バッファのサイズ。</param>
		/// <param name="srcData">ソースデータ。nullを指定することも可能。</param>
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
		/// 定数バッファの中身を更新。
		/// </summary>
		/// <param name="data"></param>
		void Update(const T* data) {
			m_innerClass->Update(data);
		}

		/// <summary>
		/// バッファの取得(DX11)
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
		/// CPUデスクリプタハンドル取得(DX12)
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
