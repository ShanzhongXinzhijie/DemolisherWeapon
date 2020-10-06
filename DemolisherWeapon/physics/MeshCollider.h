#pragma once
#include "ICollider.h"

namespace DemolisherWeapon {

	/*!
	 * @brief	メッシュコライダー。
	 */
	class MeshCollider : public ICollider
	{
	public:
		/*!
		 * @brief	コンストラクタ。
		 */
		MeshCollider();
		/*!
		 * @brief	デストラクタ。
		 */
		~MeshCollider();
		
		/*!
		 * @brief	CSkinModelからメッシュコライダーを生成。
		 *@param[in]	model		スキンモデル。
		 */
		void CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix);

		//メッシュ形状を取得
		btCollisionShape* GetBody() const override
		{
			return m_meshShape.get();
		}

		//シノニム
		typedef std::vector<CVector3>					VertexBuffer;	//頂点バッファ。
		typedef std::vector<unsigned int>				IndexBuffer;	//インデックスバッファ。
		typedef std::unique_ptr<VertexBuffer>			VertexBufferPtr;
		typedef std::unique_ptr<IndexBuffer>			IndexBufferPtr;

		//頂点バッファを取得
		std::vector<VertexBufferPtr>& GetVertexBuffer() {
			return m_vertexBufferArray;
		}

		//メッシュ形状を取得
		btBvhTriangleMeshShape* GetMeshShape() {
			return m_meshShape.get();
		}

	private:
		void CreateIndexMesh(IndexBuffer* ib, VertexBuffer* vb);

	private:		
		std::vector<VertexBufferPtr>					m_vertexBufferArray;//頂点バッファの配列。
		std::vector<IndexBufferPtr>						m_indexBufferArray;	//インデックスバッファの配列。
		std::unique_ptr<btBvhTriangleMeshShape>			m_meshShape;		//メッシュ形状。
		std::unique_ptr<btTriangleIndexVertexArray>		m_stridingMeshInterface;

		std::unique_ptr<btTriangleInfoMap>				m_triInfomap;
	};

}