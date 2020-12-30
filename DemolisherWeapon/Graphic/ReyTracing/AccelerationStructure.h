#pragma once

namespace DemolisherWeapon {

	class BLASBuffer {
	public:
		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="commandList">コマンドリスト</param>
		/// <param name="geometories">ジオメトリ</param>
		void Init(ID3D12GraphicsCommandList4* commandList, const std::vector<std::unique_ptr<ReyTracingGeometoryData>>& geometories);

		/// /// <summary>
		/// BLASBufferのリストを取得。
		/// </summary>
		/// <returns></returns>
		const std::vector<AccelerationStructureBuffers>& Get() const
		{
			return m_bottomLevelASBuffers;
		}

	private:
		std::vector<AccelerationStructureBuffers> m_bottomLevelASBuffers;
	};

	class TLASBuffer {// : public IShaderResource {
	public:
		/// <summary>
		/// TLASを構築。
		/// </summary>
		/// <param name="rc"></param>
		/// <param name="instances"></param>
		void Init(
			ID3D12GraphicsCommandList4* commandList,
			const std::list<std::unique_ptr<ReyTracingInstanceData>>& instances,
			const std::vector<AccelerationStructureBuffers>& bottomLevelASBuffers,
			bool update
		);

		/// <summary>
		/// SRVに登録。
		/// </summary>
		void CreateShaderResourceView();

		/// <summary>
		/// VRAM上の仮想アドレスを取得。
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() const {
			return m_GPUdescriptorHandle;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const {
			return m_CPUdescriptorHandle;
		}

	private:
		AccelerationStructureBuffers m_topLevelASBuffers;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUdescriptorHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUdescriptorHandle;

		StructuredBuffer<CMatrix> m_instanceWorldMatrixSB;
	};

}