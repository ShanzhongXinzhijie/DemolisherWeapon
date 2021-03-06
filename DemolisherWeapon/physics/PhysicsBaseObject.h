#pragma once

#include"ICollider.h"

namespace DemolisherWeapon {

	class MeshCollider;
	namespace GameObj { class CSkinModelRender; }

	class PhysicsBaseObject
	{
	public:
		virtual ~PhysicsBaseObject() {};

		/*!
		* @brief	ゴーストオブジェクトを解放。
		*@detail
		* 明示的なタイミングでオブジェクトを削除したい場合に呼び出してください。
		*/
		virtual void Release() = 0;

	private:
		/*!
		* @brief	静的物理オブジェクト作成処理の共通処理。
		*/
		virtual void CreateCommon(const CVector3& pos, const CQuaternion& rot) = 0;

	public:
		/*!
		 * @brief	ボックス形状のゴーストオブジェクトを作成。
		 *@param[in]	pos			座標。
		 *@param[in]	rot			回転。
		 *@param[in]	size		サイズ。
		 */
		void CreateBox(const CVector3& pos, const CQuaternion& rot, const CVector3& size);
		/*!
		 * @brief	カプセル形状のゴーストオブジェクトを作成。
		 *@param[in]	pos			座標。
		 *@param[in]	rot			回転。
		 *@param[in]	radius		カプセルの半径。
		 *@param[in]	height		カプセルの高さ。
		 */
		void CreateCapsule(const CVector3& pos, const CQuaternion& rot, float radius, float height);
		/*!
		* @brief	球体形状のゴーストオブジェクトを作成。
		*@param[in]	pos			座標。
		*@param[in]	rot			回転。
		*@param[in]	radius		球体の半径。
		*/
		void CreateSphere(const CVector3& pos, const CQuaternion& rot, float radius);
		/*!
		* @brief	メッシュ形状のゴーストオブジェクトを作成。
		*@param[in]	pos					座標。
		*@param[in]	rot					回転。
		*@param[in]	skinModelData		スキンモデルデータ。
		*/
		//void CreateMesh(CVector3 pos, CQuaternion rot, const SkinModelData& skinModelData);
		void CreateMesh(const CVector3& pos, const CQuaternion& rot, const CVector3& scale, const SkinModel& skinModel);
		void CreateMesh(const GameObj::CSkinModelRender& skinModelRender);

		/// <summary>
		/// メッシュコライダーを取得
		/// ※m_colliderがメッシュコライダーでないとnullptrが返る
		/// </summary>
		MeshCollider* GetMeshCollider();

	protected:
		std::unique_ptr<ICollider> m_collider;
	};

}