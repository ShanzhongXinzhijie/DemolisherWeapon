#pragma once
#include "RaytracingHeader.h"
#include "AccelerationStructure.h"
#include "ReyTracingDescriptorHeap.h"
#include "ReyTracingPSO.h"
#include "ShaderTable.h"

namespace DemolisherWeapon {		

	/// <summary>
	/// レイトレワールト゛
	/// </summary>
	class ReyTracingWorld
	{
	public:
		/// <summary>
		/// ジオメトリを登録。
		/// </summary>
		/// <param name="model">モデル</param>
		/// <param name="worldMatrix">ワールド行列</param>
		/// <returns>インスタンス配列の開始位置</returns>
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator RegisterModel(CModel& model, const CMatrix* worldMatrix);

		/// <summary>
		/// ジオメトリ登録を解除。
		/// </summary>
		void UnregisterModel(std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator instanceStartIndex);

		/// <summary>
		/// ジオメトリの登録を確定。
		/// (BLASとTLASを生成)
		/// </summary>
		void CommitRegisterGeometry(ID3D12GraphicsCommandList4* commandList);
		/// <summary>
		/// TLASを更新する
		/// </summary>
		void UpdateTLAS(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// レイトレワールドのインスタンスに対してクエリを行う。
		/// </summary>
		/// <param name="queryFunc"></param>
		void QueryInstances(std::function<void(ReyTracingInstanceData&)> queryFunc) const
		{
			for (auto& instance : m_instances) {
				queryFunc(*instance);
			}
		}

		/// <summary>
		/// インスタンスの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumInstance() const
		{
			return (int)m_instances.size();
		}

		/// <summary>
		/// TLAS取得
		/// </summary>
		/// <returns></returns>
		TLASBuffer& GetTLASBuffer() {
			return m_topLevelASBuffers;
		}

		/// <summary>
		/// 更新されたか
		/// </summary>
		/// <returns></returns>
		bool GetIsUpdated()const {
			return m_isUpdated;
		}

	private:
		std::list<std::unique_ptr<ReyTracingInstanceData>> m_instances;		
		std::vector<std::unique_ptr<ReyTracingGeometoryData>> m_geometories;
		BLASBuffer m_blasBuffer;
		TLASBuffer m_topLevelASBuffers;

		bool m_isUpdated = false;
	};	

	/// <summary>
	/// レイトレーシングエンジン
	/// </summary>
	class RayTracingEngine
	{
	public:
		/// <summary>
		/// レイトレーシングをディスパッチ。
		/// </summary>
		/// <param name="commandList">コマンドリスト</param>
		void Dispatch(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// モデルを登録
		/// </summary>
		/// <param name="model">モデル</param>
		/// <param name="worldMatrix">ワールド行列</param>
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator RegisterModel(CModel& model, const CMatrix* worldMatrix)
		{
			return m_world.RegisterModel(model, worldMatrix);
		}
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator RegisterModel(SkinModel& model)
		{
			return m_world.RegisterModel(*model.GetModel(), &model.GetWorldMatrix());
		}

		/// <summary>
		/// モデル登録を解除
		/// </summary>
		void UnregisterModel(std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator startItr) {
			m_world.UnregisterModel(startItr);
		}

		/// <summary>
		/// 更新処理
		/// </summary>
		void Update(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// TLASのみの更新
		/// </summary>
		void UpdateTLAS(ID3D12GraphicsCommandList4* commandList) {
			m_world.UpdateTLAS(commandList);
		}

		/// <summary>
		/// 定数バッファ取得
		/// </summary>
		/// <returns></returns>
		ConstantBuffer<ReyTracingCBStructure>& GetCB() {
			return m_rayGenerationCB;
		}

	private:
		/// <summary>
		/// シェーダーリソースを作成。
		/// </summary>
		void CreateShaderResources();

	private:
		bool m_isCommit = false;

		ReyTracingWorld m_world;
		ReyTracingPSO m_pipelineStateObject;
		ShaderTable m_shaderTable;
		ReyTracingDescriptorHeap m_descriptorHeap;

		ConstantBuffer<ReyTracingCBStructure> m_rayGenerationCB;//定数バッファ
		
		Microsoft::WRL::ComPtr<ID3D12Resource> m_raytracingOutput;				//出力バッファ
		D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;//出力バッファのGPUディスクリプタハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVCpuDescriptor;//出力バッファのCPUディスクリプタハンドル
	};

}
