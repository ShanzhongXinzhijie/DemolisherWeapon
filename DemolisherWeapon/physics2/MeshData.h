#pragma once

namespace DemolisherWeapon {
	namespace Physics2 {

		class MeshData
		{
		public:
			typedef std::vector<CVector3>					VertexBuffer;
			typedef std::vector<unsigned int>				Index;
			typedef std::vector<Index>						IndexBuffer;

		public:
			MeshData() = default;
			~MeshData() = default;

			void Reset() {
				m_vertexBuffer.clear();
				m_indexBuffer.clear();
			}

			/// <summary>
			/// �X�L�����f�����烁�b�V���f�[�^�̍쐬
			/// </summary>
			/// <param name="model">���ƂȂ�X�L�����f��</param>
			/// <param name="offsetMatrix">�I�t�Z�b�g�s��</param>
			void CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix);

			/// <summary>
			/// ���_�o�b�t�@�̎擾
			/// </summary>
			/// <returns></returns>
			const VertexBuffer& GetVertexBuffer()const { return m_vertexBuffer; }
			/// <summary>
			/// �|���S��(�C���f�b�N�X)���̎擾
			/// </summary>
			/// <returns></returns>
			size_t GetPolygonNum()const { return m_indexBuffer.size(); }
			/// <summary>
			/// �ꖇ�̃|���S���̃C���f�b�N�X���擾
			/// </summary>
			/// <param name="index">�����ڂ̃|���S����</param>
			/// <returns></returns>
			const Index& GetPolygonIndexs(int index)const { return m_indexBuffer[index]; }

		private:			
			VertexBuffer	m_vertexBuffer;
			IndexBuffer		m_indexBuffer;
		};

	}
}