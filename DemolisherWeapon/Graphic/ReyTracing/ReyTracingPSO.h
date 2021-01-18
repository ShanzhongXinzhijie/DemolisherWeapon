#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// レイトレ用のパイプラインステートオブジェクト。
	/// </summary>
	class ReyTracingPSO {
	private:
		/// <summary>
		/// ルートシグネチャ定義。
		/// </summary>
		struct RootSignatureDesc
		{
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			std::vector<D3D12_DESCRIPTOR_RANGE> range;
			std::vector<D3D12_ROOT_PARAMETER> rootParams;
		};

	public:
		/// <summary>
		/// パイプラインステートオブジェクトを初期化。
		/// </summary>
		void Init(const ReyTracingDescriptorHeap* DH);

		void QueryInterface(Microsoft::WRL::ComPtr<ID3D12StateObjectProperties>& props) const
		{
			m_pipelineState.As(&props);
			//m_pipelineState->QueryInterface(&props);
		}

		/// <summary>
		/// グローバルルートシグネチャを取得。
		/// </summary>
		/// <returns></returns>
		ID3D12RootSignature* GetGlobalRootSignature()
		{
			return m_emptyRootSignature.Get();
		}

		ID3D12StateObject* Get()
		{
			return m_pipelineState.Get();
		}

	private:
		RootSignatureDesc CreateRayGenRootSignatureDesc();
		RootSignatureDesc CreatePBRMatterialHitRootSignatureDesc();

	private:
		const ReyTracingDescriptorHeap* m_descriptorHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12StateObject> m_pipelineState;					//パイプラインステート
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_emptyRootSignature;
	};

}