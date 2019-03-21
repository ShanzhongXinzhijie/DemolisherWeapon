/*!
 * @brief	静的オブジェクト。
 */

#pragma once

#include "MeshCollider.h"
#include "RigidBody.h"
#include "PhysicsBaseObject.h"

namespace DemolisherWeapon {

	/*!
	* @brief	静的物理オブジェクト
	*/
	class PhysicsStaticObject : public PhysicsBaseObject {
	public:
		~PhysicsStaticObject()
		{
			Release();
		}
		/*!
		* @brief	解放。
		*/
		void Release() override final;
		/*!
		* @brief	座標と回転を設定。
		*/
		void SetPositionAndRotation(const CVector3& pos, const CQuaternion& rot)
		{
			m_rigidBody.SetPositionAndRotation(pos, rot);
		}
		RigidBody& GetRigidBody()
		{
			return m_rigidBody;
		}
	private:

		/*!
		* @brief	静的物理オブジェクト作成処理の共通処理。
		*/
		void CreateCommon(CVector3 pos, CQuaternion rot) override final;
	private:
		bool m_isAdd = false;
		RigidBody m_rigidBody;				//!<剛体。
	};

}