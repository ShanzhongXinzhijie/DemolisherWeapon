#pragma once

#include "RigidBody.h"
#include "PhysicsBaseObject.h"

namespace DemolisherWeapon {

	/*!
	* @brief	動的物理オブジェクト
	*@details
	* CPhysicsObjectBaseのpublicパブリックメンバ関数を
	* 実装のために使っており、外部に公開したくなかったため
	* private継承を行っている。
	*/
	class PhysicsDynamicObject :
		public PhysicsBaseObject
	{
	public:
		/*!
		* @brief	ゴーストオブジェクトを解放。
		*@detail
		* 明示的なタイミングでオブジェクトを削除したい場合に呼び出してください。
		*/
		void Release() override final;
	private:
		/*!
		* @brief	静的物理オブジェクト作成処理の共通処理。
		*/
		void CreateCommon(CVector3 pos, CQuaternion rot) override final;
	public:
		/*!
		 * @brief	デストラクタ。
		 */
		~PhysicsDynamicObject()
		{
			Release();
		}
		/*!
		* @brief	ボックス形状のゴーストオブジェクトを作成。
		*@param[in]	pos			座標。
		*@param[in]	rot			回転。
		*@param[in]	size		サイズ。
		*@param[in] mass		質量。
		*/
		void CreateBox(CVector3 pos, CQuaternion rot, CVector3 size, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateBox(pos, rot, size);
		}
		/*!
		* @brief	カプセル形状のゴーストオブジェクトを作成。
		*@param[in]	pos			座標。
		*@param[in]	rot			回転。
		*@param[in]	radius		カプセルの半径。
		*@param[in]	height		カプセルの高さ。
		*@param[in]	mass		質量。
		*/
		void CreateCapsule(CVector3 pos, CQuaternion rot, float radius, float height, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateCapsule(pos, rot, radius, height);
		}
		/*!
		* @brief	球体形状のゴーストオブジェクトを作成。
		*@param[in]	pos			座標。
		*@param[in]	rot			回転。
		*@param[in]	radius		球体の半径。
		*@param[in]	mass		質量。
		*/
		void CreateSphere(CVector3 pos, CQuaternion rot, float radius, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateSphere(pos, rot, radius);
		}
		/*!
		* @brief	メッシュ形状のゴーストオブジェクトを作成。
		*@param[in]	pos					座標。
		*@param[in]	rot					回転。
		*@param[in]	skinModelData		スキンモデルデータ。
		*@param[in] mass				質量。
		*/
		/*void CreateMesh(CVector3 pos, CQuaternion rot, const SkinModelData& skinModelData, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateMesh(pos, rot, skinModelData);
		}*/
		void CreateMesh(CVector3 pos, CQuaternion rot, CVector3 scale, const SkinModel& skinModel, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateMesh(pos, rot, scale, skinModel);
		}
		void CreateMesh(CVector3 pos, CQuaternion rot, CVector3 scale, GameObj::CSkinModelRender* skinModelRender, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateMesh(*skinModelRender);
		}
		/*!
		* @brief	物理オブジェクトの座標と回転を取得
		*@param[out]	pos			座標。
		*@param[out]	rot			回転。
		*/
		void GetPositionAndRotation(CVector3& pos, CQuaternion& rot) const
		{
			m_rigidBody.GetPositionAndRotation(pos, rot);
		}
		/*!
		* @brief	力を加える。
		*@param[out]	force		力。
		*@param[out]	relPos		力を加える座標。
		*/
		void AddForce(const CVector3& force, const CVector3& relPos = CVector3::Zero())
		{
			m_rigidBody.AddForce(force, relPos);
		}
		RigidBody& GetRigidBody()
		{
			return m_rigidBody;
		}
	private:
		CVector3 m_localInterial = CVector3::Zero();	//!<慣性テンソ。ル
		float m_mass = 1.0f;				//!<質量。
		RigidBody m_rigidBody;				//!<剛体。
	};

}