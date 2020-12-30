#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// �V�F�[�_�[�e�[�u��
	/// </summary>
	/// <remark>
	/// �V�F�[�_�[�e�[�u���̓��C�̃f�B�X�p�b�`�Ŏg�p�����A
	/// �V�F�[�_�[�⃊�\�[�X�̃f�B�X�N���v�^�q�[�v�̃A�h���X�Ȃǂ��o�^����Ă���e�[�u���ł��B
	/// </remark>
	class ShaderTable {
	public:
		/// <summary>
		/// �V�F�[�_�[�e�[�u�����������B
		/// </summary>
		void Init(
			const ReyTracingWorld& world,
			const ReyTracingPSO& pso,
			const ReyTracingDescriptorHeap& descriptorHeaps
		);

		/// <summary>
		/// �V�F�[�_�[�e�[�u����GPU��̉��z�A�h���X���擾�B
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
		{
			return m_shaderTable->GetGPUVirtualAddress();
		}
		/// <summary>
		/// �V�F�[�_�[�e�[�u���ɋL������Ă���f�[�^�̂P�v�f�̃T�C�Y���擾�B
		/// </summary>
		/// <returns></returns>
		uint32_t GetShaderTableEntrySize() const
		{
			return m_shaderTableEntrySize;
		}
		/// <summary>
		/// ���C�W�F�l���[�V�����V�F�[�_�[�̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumRayGenShader() const
		{
			return m_numRayGenShader;
		}
		/// <summary>
		/// �~�X�V�F�[�_�[�̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumMissShader() const
		{
			return m_numMissShader;
		}
		/// <summary>
		/// �V�F�[�_�[�e�[�u���ɓo�^����Ă���q�b�g�V�F�[�_�[�̐����擾�B
		/// </summary>
		/// <remark>
		/// �q�b�g�V�F�[�_�[�̐��̓C���X�^���X�̐��Ɠ����ɂȂ�܂��B
		/// </remark>
		/// <returns></returns>
		int GetNumHitShader() const
		{
			return m_numHitShader;
		}
	private:
		/// <summary>
		/// ���C�W�F�l���[�V�����V�F�[�_�[�A�~�X�V�F�[�_�[�A�q�b�g�V�F�[�_�[�̐����J�E���g����B
		/// </summary>
		void CountupNumGeyGenAndMissAndHitShader();
		/// <summary>
		/// �V�F�[�_�[�e�[�u����1�v�f�̃T�C�Y���v�Z����B
		/// </summary>
		void CalcShaderTableEntrySize();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_shaderTable;			//�V�F�[�_�[�e�[�u���B
		uint32_t m_shaderTableEntrySize = 0;
		int m_numRayGenShader = 0;
		int m_numMissShader = 0;
		int m_numHitShader = 0;
	};

}