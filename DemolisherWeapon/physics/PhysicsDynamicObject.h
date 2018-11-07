#pragma once

#include "RigidBody.h"
#include "PhysicsBaseObject.h"

namespace DemolisherWeapon {

	/*!
	* @brief	���I�����I�u�W�F�N�g
	*@details
	* CPhysicsObjectBase��public�p�u���b�N�����o�֐���
	* �����̂��߂Ɏg���Ă���A�O���Ɍ��J�������Ȃ���������
	* private�p�����s���Ă���B
	*/
	class PhysicsDynamicObject :
		public PhysicsBaseObject
	{
	public:
		/*!
		* @brief	�S�[�X�g�I�u�W�F�N�g������B
		*@detail
		* �����I�ȃ^�C�~���O�ŃI�u�W�F�N�g���폜�������ꍇ�ɌĂяo���Ă��������B
		*/
		void Release() override final;
	private:
		/*!
		* @brief	�ÓI�����I�u�W�F�N�g�쐬�����̋��ʏ����B
		*/
		void CreateCommon(CVector3 pos, CQuaternion rot) override final;
	public:
		/*!
		 * @brief	�f�X�g���N�^�B
		 */
		~PhysicsDynamicObject()
		{
			Release();
		}
		/*!
		* @brief	�{�b�N�X�`��̃S�[�X�g�I�u�W�F�N�g���쐬�B
		*@param[in]	pos			���W�B
		*@param[in]	rot			��]�B
		*@param[in]	size		�T�C�Y�B
		*@param[in] mass		���ʁB
		*/
		void CreateBox(CVector3 pos, CQuaternion rot, CVector3 size, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateBox(pos, rot, size);
		}
		/*!
		* @brief	�J�v�Z���`��̃S�[�X�g�I�u�W�F�N�g���쐬�B
		*@param[in]	pos			���W�B
		*@param[in]	rot			��]�B
		*@param[in]	radius		�J�v�Z���̔��a�B
		*@param[in]	height		�J�v�Z���̍����B
		*@param[in]	mass		���ʁB
		*/
		void CreateCapsule(CVector3 pos, CQuaternion rot, float radius, float height, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateCapsule(pos, rot, radius, height);
		}
		/*!
		* @brief	���̌`��̃S�[�X�g�I�u�W�F�N�g���쐬�B
		*@param[in]	pos			���W�B
		*@param[in]	rot			��]�B
		*@param[in]	radius		���̂̔��a�B
		*@param[in]	mass		���ʁB
		*/
		void CreateSphere(CVector3 pos, CQuaternion rot, float radius, float mass, CVector3 localInteria)
		{
			m_mass = mass;
			m_localInterial = localInteria;
			PhysicsBaseObject::CreateSphere(pos, rot, radius);
		}
		/*!
		* @brief	���b�V���`��̃S�[�X�g�I�u�W�F�N�g���쐬�B
		*@param[in]	pos					���W�B
		*@param[in]	rot					��]�B
		*@param[in]	skinModelData		�X�L�����f���f�[�^�B
		*@param[in] mass				���ʁB
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
		* @brief	�����I�u�W�F�N�g�̍��W�Ɖ�]���擾
		*@param[out]	pos			���W�B
		*@param[out]	rot			��]�B
		*/
		void GetPositionAndRotation(CVector3& pos, CQuaternion& rot) const
		{
			m_rigidBody.GetPositionAndRotation(pos, rot);
		}
		/*!
		* @brief	�͂�������B
		*@param[out]	force		�́B
		*@param[out]	relPos		�͂���������W�B
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
		CVector3 m_localInterial = CVector3::Zero();	//!<�����e���\�B��
		float m_mass = 1.0f;				//!<���ʁB
		RigidBody m_rigidBody;				//!<���́B
	};

}