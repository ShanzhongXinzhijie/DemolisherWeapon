#pragma once
#include "RaytracingHeader.h"
#include "AccelerationStructure.h"
#include "ReyTracingDescriptorHeap.h"
#include "ReyTracingPSO.h"
#include "ShaderTable.h"

namespace DemolisherWeapon {		

	/// <summary>
	/// ���C�g�����[���g�J
	/// </summary>
	class ReyTracingWorld
	{
	public:
		/// <summary>
		/// �W�I���g����o�^�B
		/// </summary>
		/// <param name="model">���f��</param>
		/// <param name="worldMatrix">���[���h�s��</param>
		/// <returns>�C���X�^���X�z��̊J�n�ʒu</returns>
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator RegisterModel(CModel& model, const CMatrix* worldMatrix);

		/// <summary>
		/// �W�I���g���o�^�������B
		/// </summary>
		void UnregisterModel(std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator instanceStartIndex);

		/// <summary>
		/// �W�I���g���̓o�^���m��B
		/// (BLAS��TLAS�𐶐�)
		/// </summary>
		void CommitRegisterGeometry(ID3D12GraphicsCommandList4* commandList);
		/// <summary>
		/// TLAS���X�V����
		/// </summary>
		void UpdateTLAS(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// ���C�g�����[���h�̃C���X�^���X�ɑ΂��ăN�G�����s���B
		/// </summary>
		/// <param name="queryFunc"></param>
		void QueryInstances(std::function<void(ReyTracingInstanceData&)> queryFunc) const
		{
			for (auto& instance : m_instances) {
				queryFunc(*instance);
			}
		}

		/// <summary>
		/// �C���X�^���X�̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumInstance() const
		{
			return (int)m_instances.size();
		}

		/// <summary>
		/// TLAS�擾
		/// </summary>
		/// <returns></returns>
		TLASBuffer& GetTLASBuffer() {
			return m_topLevelASBuffers;
		}

		/// <summary>
		/// �X�V���ꂽ��
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
	/// ���C�g���[�V���O�G���W��
	/// </summary>
	class RayTracingEngine
	{
	public:
		/// <summary>
		/// ���C�g���[�V���O���f�B�X�p�b�`�B
		/// </summary>
		/// <param name="commandList">�R�}���h���X�g</param>
		void Dispatch(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// ���f����o�^
		/// </summary>
		/// <param name="model">���f��</param>
		/// <param name="worldMatrix">���[���h�s��</param>
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator RegisterModel(CModel& model, const CMatrix* worldMatrix)
		{
			return m_world.RegisterModel(model, worldMatrix);
		}
		std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator RegisterModel(SkinModel& model)
		{
			return m_world.RegisterModel(*model.GetModel(), &model.GetWorldMatrix());
		}

		/// <summary>
		/// ���f���o�^������
		/// </summary>
		void UnregisterModel(std::list<std::unique_ptr<ReyTracingInstanceData>>::iterator startItr) {
			m_world.UnregisterModel(startItr);
		}

		/// <summary>
		/// �X�V����
		/// </summary>
		void Update(ID3D12GraphicsCommandList4* commandList);

		/// <summary>
		/// TLAS�݂̂̍X�V
		/// </summary>
		void UpdateTLAS(ID3D12GraphicsCommandList4* commandList) {
			m_world.UpdateTLAS(commandList);
		}

		/// <summary>
		/// �萔�o�b�t�@�擾
		/// </summary>
		/// <returns></returns>
		ConstantBuffer<ReyTracingCBStructure>& GetCB() {
			return m_rayGenerationCB;
		}

	private:
		/// <summary>
		/// �V�F�[�_�[���\�[�X���쐬�B
		/// </summary>
		void CreateShaderResources();

	private:
		bool m_isCommit = false;

		ReyTracingWorld m_world;
		ReyTracingPSO m_pipelineStateObject;
		ShaderTable m_shaderTable;
		ReyTracingDescriptorHeap m_descriptorHeap;

		ConstantBuffer<ReyTracingCBStructure> m_rayGenerationCB;//�萔�o�b�t�@
		
		Microsoft::WRL::ComPtr<ID3D12Resource> m_raytracingOutput;				//�o�̓o�b�t�@
		D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;//�o�̓o�b�t�@��GPU�f�B�X�N���v�^�n���h��
		D3D12_CPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVCpuDescriptor;//�o�̓o�b�t�@��CPU�f�B�X�N���v�^�n���h��
	};

}
