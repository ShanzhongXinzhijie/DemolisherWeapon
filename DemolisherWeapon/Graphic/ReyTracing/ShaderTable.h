#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// シェーダーテーブル
	/// </summary>
	/// <remark>
	/// シェーダーテーブルはレイのディスパッチで使用される、
	/// シェーダーやリソースのディスクリプタヒープのアドレスなどが登録されているテーブルです。
	/// </remark>
	class ShaderTable {
	public:
		/// <summary>
		/// シェーダーテーブルを初期化。
		/// </summary>
		void Init(
			const ReyTracingWorld& world,
			const ReyTracingPSO& pso,
			const ReyTracingDescriptorHeap& descriptorHeaps
		);

		/// <summary>
		/// シェーダーテーブルのGPU上の仮想アドレスを取得。
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_shaderTable->GetGPUVirtualAddress();
		}
		/// <summary>
		/// シェーダーテーブルに記憶されているデータの１要素のサイズを取得。
		/// </summary>
		/// <returns></returns>
		uint32_t GetShaderTableEntrySize() const
		{
			return m_shaderTableEntrySize;
		}
		/// <summary>
		/// レイジェネレーションシェーダーの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumRayGenShader() const
		{
			return m_numRayGenShader;
		}
		/// <summary>
		/// ミスシェーダーの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumMissShader() const
		{
			return m_numMissShader;
		}
		/// <summary>
		/// シェーダーテーブルに登録されているヒットシェーダーの数を取得。
		/// </summary>
		/// <remark>
		/// ヒットシェーダーの数はインスタンスの数と同じになります。
		/// </remark>
		/// <returns></returns>
		int GetNumHitShader() const
		{
			return m_numHitShader;
		}
	private:
		/// <summary>
		/// レイジェネレーションシェーダー、ミスシェーダー、ヒットシェーダーの数をカウントする。
		/// </summary>
		void CountupNumGeyGenAndMissAndHitShader();
		/// <summary>
		/// シェーダーテーブルの1要素のサイズを計算する。
		/// </summary>
		void CalcShaderTableEntrySize();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_shaderTable;			//シェーダーテーブル。
		uint32_t m_shaderTableEntrySize = 0;
		int m_numRayGenShader = 0;
		int m_numMissShader = 0;
		int m_numHitShader = 0;
	};

}