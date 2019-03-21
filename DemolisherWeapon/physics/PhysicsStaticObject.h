/*!
 * @brief	�ÓI�I�u�W�F�N�g�B
 */

#pragma once

#include "MeshCollider.h"
#include "RigidBody.h"
#include "PhysicsBaseObject.h"

namespace DemolisherWeapon {

	/*!
	* @brief	�ÓI�����I�u�W�F�N�g
	*/
	class PhysicsStaticObject : public PhysicsBaseObject {
	public:
		~PhysicsStaticObject()
		{
			Release();
		}
		/*!
		* @brief	����B
		*/
		void Release() override final;
		/*!
		* @brief	���W�Ɖ�]��ݒ�B
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
		* @brief	�ÓI�����I�u�W�F�N�g�쐬�����̋��ʏ����B
		*/
		void CreateCommon(CVector3 pos, CQuaternion rot) override final;
	private:
		bool m_isAdd = false;
		RigidBody m_rigidBody;				//!<���́B
	};

}