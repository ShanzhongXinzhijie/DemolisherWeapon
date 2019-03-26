#pragma once

namespace DemolisherWeapon {
	namespace Physics2 {

		class MeshData
		{
		public:
			MeshData() = default;
			~MeshData() = default;

			void Reset() {
				m_vertexBuffer.clear();
				m_indexBuffer.clear();
			}

			void CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix);

		private:
			typedef std::vector<CVector3>					VertexBuffer;
			typedef std::vector<unsigned int>				IndexBuffer;

			VertexBuffer	m_vertexBuffer;
			IndexBuffer		m_indexBuffer;
		};

	}
}