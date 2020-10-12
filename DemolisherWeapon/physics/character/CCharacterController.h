#pragma once

#include "../SphereCollider.h"
#include "../CapsuleCollider.h"
#include "../RigidBody.h"

namespace DemolisherWeapon {
	/*!
	 * @brief	�L�����N�^�R���g���[���[�B
	 *@details
	 * �L�����N�^�[�Ɣw�i�̏Փˉ������s�����Ƃ��ł��܂��B\n
	 * ���L�̃T���v���v���O�������Q�l�ɂ��Ă��������B\n
	 * Sample/Sample07
	 */
	class CCharacterController {
	public:
		CCharacterController(bool colGhost = false) : m_isColGhost(colGhost){

		}
		~CCharacterController()
		{
			RemoveRigidBoby();
		}
		/*!
		 * @brief	�������B
		 *@param[in]	radius		�J�v�Z���R���C�_�[�̔��a�B
		 *@param[in]	height		�J�v�Z���R���C�_�[�̍����B
		 *@param[in]	position	�����ʒu�B
		 */
		void Init(float radius, float height, const CVector3& position);
		/*!
		 * @brief	���s�B
		 *@param[in, out]	moveSpeed		�ړ����x�B
		 *@param[in]	deltaTime		�o�ߎ��ԁB�P�ʂ͕b�B�f�t�H���g�ł́A�P�t���[���̌o�ߎ��Ԃ��n����Ă��܂��B
		 *@return �ړ���̃L�����N�^�[�̍��W�B
		 */
		const CVector3& Execute(CVector3& moveSpeed, float deltaTime = 1.0f / GetStandardFrameRate());
		/*!
		 * @brief	���W���擾�B
		 */
		const CVector3& GetPosition() const
		{
			return m_position;
		}
		/*!
		 * @brief	���W��ݒ�B
		 */
		void SetPosition(const CVector3& pos)
		{
			m_position = pos;
		}

		/*!
		 * @brief	�W�����v��������
		 */
		bool IsJump() const
		{
			return m_isJump;
		}
		/*!
		* @brief	�n�ʏ�ɂ��邩����B
		*/
		bool IsOnGround() const
		{
			return m_isOnGround;
		}
		//�ǂɐڐG���Ă��邩�H
		bool IsContactWall() const
		{
			return m_isContactWall;
		}
		//�ڐG���Ă���ǂ̖@��(����)���擾
		const CVector3& GetContactWallNormal()const {
			return m_contactWallNormal;			
		}
		/*!
		* @brief	�R���C�_�[���擾�B
		*/
		CapsuleCollider* GetCollider()
		{
			return &m_collider;
		}
		/*!
		* @brief	���̂��擾�B
		*/
		RigidBody* GetRigidBody()
		{
			return &m_rigidBody;
		}
		/*!
		* @brief	���̂𕨗��G���W������폜�B�B
		*/
		void RemoveRigidBoby();
		/// <summary>
		/// ���̂𕨗��G���W���ɒǉ�
		/// </summary>
		void AddRigidBoby();

		//�ǂ���]���ɉ����o��������ݒ�
		void SetOffset(float offset) {
			m_offsetOffset = offset;
		}

	private:
		bool				m_isInited = false;				//!<�������ς݁H
		CVector3 			m_position = CVector3::Zero();	//!<���W�B
		bool 				m_isJump = false;				//!<�W�����v���H
		bool				m_isOnGround = true;			//!<�n�ʂ̏�ɂ���H
		bool				m_isContactWall = false;		//�ǂɐڐG���Ă�H
		CVector3			m_contactWallNormal;			//�ڐG���Ă���ǂ̖@��(����)
		CapsuleCollider		m_collider;						//!<�R���C�_�[�B
		CapsuleCollider		m_colliderWall;
		float				m_radius = 0.0f;				//!<�J�v�Z���R���C�_�[�̔��a�B
		float				m_height = 0.0f;				//!<�J�v�Z���R���C�_�[�̍����B
		RigidBody			m_rigidBody;					//���́B

		float m_offsetOffset = 1.0f;

		bool m_isColGhost = false;//�S�[�X�g�I�u�W�F�N�g�ƏՓ˂��邩
	};

	//�S�[�X�g�ƏՓ˂���L�����R��
	class CCharacterControllerType2 : public CCharacterController {
	public:
		CCharacterControllerType2() : CCharacterController(true) {}
	};
}