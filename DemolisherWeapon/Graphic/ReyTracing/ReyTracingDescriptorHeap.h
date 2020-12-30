#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// レイトレのディスクリプタヒープ
	/// </summary>
	class ReyTracingDescriptorHeap {
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		void Init(
			ReyTracingWorld& world,
			ConstantBuffer<ReyTracingCBStructure>& cb,
			D3D12_CPU_DESCRIPTOR_HANDLE uavHandle
		);

		/// <summary>
		/// 更新
		/// </summary>
		int Update(ReyTracingWorld& world);

		//ディスクリプタヒープ取得
		ID3D12DescriptorHeap* GetSRVHeap()const {
			return m_srvsDescriptorHeap.Get();
		}
		ID3D12DescriptorHeap* GetSamplerHeap()const {
			return m_samplerDescriptorHeap.Get();
		}

		/// <summary>
		/// SRVデスクリプタのサイズ
		/// </summary>
		/// <returns></returns>
		UINT GetSrvsDescriptorSize()const {
			return m_srvsDescriptorSize;
		}

		/// <summary>
		/// UAVデスクリプタの開始位置
		/// </summary>
		/// <returns></returns>
		int GetOffsetUAVDescriptorFromTableStart()const {
			return m_uavStartNum;
		}
		/// <summary>
		/// SRVデスクリプタの開始位置
		/// </summary>
		/// <returns></returns>
		int GetOffsetSRVDescriptorFromTableStart()const {
			return m_srvStartNum;
		}
		/// <summary>
		/// CBVデスクリプタの開始位置
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