#pragma once

#include"ICollider.h"

namespace DemolisherWeapon {

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
		virtual void CreateCommon(CVector3 pos, CQuaternion rot) = 0;

	public:
		/*!
		 * @brief	ボックス形状のゴーストオブジェクトを作成。
		 *@param[in]	pos			座標。
		 *@param[in]	rot			回転。
		 *@param[in]	size		サイズ。
		 */
		void CreateBox(CVector3 pos, CQuaternion rot, CVector3 size);
		/*!
		 * @brief	カプセル形状のゴーストオブジェクトを作成。
		 *@param[in]	pos			座標。
		 *@param[in]	rot			回転。
		 *@param[in]	radius		カプセルの半径。
		 *@param[in]	height		カプセルの高さ。
		 */
		void CreateCapsule(CVector3 pos, CQuaternion rot, float radius, float height);
		/*!
		* @brief	球体形状のゴーストオブジェクトを作成。
		*@param[in]	pos			座標。
		*@param[in]	rot			回転。
		*@param[in]	radius		球体の半径。
		*/
		void CreateSphere(CVector3 pos, CQuaternion rot, float radius);
		/*!
		* @brief	メッシュ形状のゴーストオブジェクトを作成。
		*@param[in]	pos					座標。
		*@param[in]	rot					回転。
		*@param[in]	skinModelData		スキンモデルデータ。
		*/
		//void CreateMesh(CVector3 pos, CQuaternion rot, const SkinModelData& skinModelData);
		void CreateMesh(CVector3 pos, CQuaternion rot, CVector3 scale, const SkinModel& skinModel);
		void CreateMesh(const GameObj::CSkinModelRender& skinModelRender);

	protected:
		std::unique_ptr<ICollider> m_collider;
	};

}