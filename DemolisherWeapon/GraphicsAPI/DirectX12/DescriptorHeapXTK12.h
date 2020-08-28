#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// DirectXTK12のディスクリプタヒープを扱うクラス
	/// </summary>
	class DescriptorHeapXTK12 {
	public:
		void Init(ID3D12Device* device, size_t count) {
			m_resourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(device, count);
		}
		void Release() {
			m_nextDescriptorNum = 0;
			m_resourceDescriptors.reset();
		}

		int CreateDescriptorNumber(D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {
			m_nextDescriptorNum++;
			cpuHandle = m_resourceDescriptors->GetCpuHandle(m_nextDescriptorNum);
			gpuHandle = m_resourceDescriptors->GetGpuHandle(m_nextDescriptorNum);
			return m_nextDescriptorNum;
		}

		auto Heap() {
			return m_resourceDescriptors->Heap();
		}
		//auto GetCpuHandle(int num);
		//auto GetGpuHandle(int num);

	private:
		int m_nextDescriptorNum = 0;
		std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
	};

}