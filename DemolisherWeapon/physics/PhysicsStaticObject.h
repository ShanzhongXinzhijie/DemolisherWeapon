/*!
 * @brief	�ÓI�I�u�W�F�N�g�B
 */

#pragma once

#include "MeshCollider.h"
#include "RigidBody.h"

namespace DemolisherWeapon {

/*!
* @brief	�ÓI�����I�u�W�F�N�g
*/
class PhysicsStaticObject{
public:
	/*!
		* @brief	�R���X�g���N�^�B
		*/
	PhysicsStaticObject();
	/*!
		* @brief	�f�X�g���N�^�B
		*/
	~PhysicsStaticObject();
	/*!
		* @brief	���b�V���̐ÓI�I�u�W�F�N�g���쐬�B
		*@param[in]	skinModel	�X�L�����f���B
		*@param[in]	pos			���W�B
		*@param[in]	rot			��]�B
		*/
	void CreateMeshObject(SkinModel& skinModel, CVector3 pos, CQuaternion rot, const CMatrix* offsetMatrix = nullptr);
	
private:
	MeshCollider m_meshCollider;		//!<���b�V���R���C�_�[�B
	RigidBody m_rigidBody;				//!<���́B
};

}