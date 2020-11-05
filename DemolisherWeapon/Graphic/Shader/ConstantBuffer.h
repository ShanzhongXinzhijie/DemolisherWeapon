#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// 定数バッファ
	/// バックバッファの数だけいる...
	/// </summary>
	template<class T>
	class ConstantBufferDx12 {
	public:
		ConstantBufferDx12() = default;

		/// <summary>
		/// デストラクタ。
		/// </summary>
		~ConstantBufferDx12() {
			CD3DX12_RANGE readRange(0, 0);
			m_constantBuffer->Unmap(0, &readRange);
		}

		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="size">定数バッファのサイズ。</param>
		/// <param name="srcData">ソースデータ。nullを指定することも可能。</param>
		void Init(int size, T* srcData = nullptr) {
			m_size = size;
			//定数バッファは256バイトアライメントが要求されるので、256の倍数に切り上げる。
			m_allocSize = (size + 256) & 0xFFFFFF00;
			//定数バッファの作成。
			GetGraphicsEngine().GetD3D12Device()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_allocSize),
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
		}

		/// <summary>
		/// 定数バッファの中身を更新。
		/// </summary>
		/// <param name="data"></param>
		void Update(T* data)  {
			memcpy(m_constBufferCPU, data, m_size);
		}

		/// <summary>
		/// ConstantBufferViewの作成
		/// </summary>
		/// <param name="descriptorHandle">ディスクリプタのハンドル</param>
		/*void CreateConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle){
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
			desc.SizeInBytes = m_allocSize;
			GetGraphicsEngine().GetD3D12Device()->CreateConstantBufferView(&desc, descriptorHandle);
		}*/

		/// <summary>
		/// ConstantBufferViewの作成
		/// </summary>
		void CreateConstantBufferView() {
			auto [gpu,cpu] = GetGraphicsEngine().GetDX12().CreateConstantBufferView(m_constantBuffer.Get(), m_allocSize);
			m_cpuHandle = cpu;
			m_gpuHandle = gpu;
		}

		/// <summary>
		/// CPUデスクリプタハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() {
			return m_cpuHandle;
		}
		/// <summary>
		/// GPUデスクリプタハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() {
			return m_gpuHandle;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_constantBuffer;	//定数バッファ。
		T* m_constBufferCPU = nullptr;								//CPU側からアクセスできるする定数バッファのアドレス。
		int m_size = 0;												//定数バッファのサイズ。
		int m_allocSize = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

}
