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
			/// スキンモデルからメッシュデータの作成
			/// </summary>
			/// <param name="model">元となるスキンモデル</param>
			/// <param name="offsetMatrix">オフセット行列</param>
			void CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix);

			/// <summary>
			/// 頂点バッファの取得
			/// </summary>
			/// <returns></returns>
			const VertexBuffer& GetVertexBuffer()const { return m_vertexBuffer; }
			/// <summary>
			/// ポリゴン(インデックス)数の取得
			/// </summary>
			/// <returns></returns>
			size_t GetPolygonNum()const { return m_indexBuffer.size(); }
			/// <summary>
			/// 一枚のポリゴンのインデックスを取得
			/// </summary>
			/// <param name="index">何枚目のポリゴンか</param>
			/// <returns></returns>
			const Index& GetPolygonIndexs(int index)const { return m_indexBuffer[index]; }

		private:			
			VertexBuffer	m_vertexBuffer;
			IndexBuffer		m_indexBuffer;
		};

	}
}