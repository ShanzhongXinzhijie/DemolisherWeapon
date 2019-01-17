#pragma once

#include "../SphereCollider.h"
#include "../CapsuleCollider.h"
#include "../RigidBody.h"

namespace DemolisherWeapon {
	/*!
	 * @brief	キャラクタコントローラー。
	 *@details
	 * キャラクターと背景の衝突解決を行うことができます。\n
	 * 下記のサンプルプログラムを参考にしてください。\n
	 * Sample/Sample07
	 */
	class CCharacterController {
	public:
		CCharacterController() {

		}
		~CCharacterController()
		{
			RemoveRigidBoby();
		}
		/*!
		 * @brief	初期化。
		 *@param[in]	radius		カプセルコライダーの半径。
		 *@param[in]	height		カプセルコライダーの高さ。
		 *@param[in]	position	初期位置。
		 */
		void Init(float radius, float height, const CVector3& position);
		/*!
		 * @brief	実行。
		 *@param[in, out]	moveSpeed		移動速度。
		 *@param[in]	deltaTime		経過時間。単位は秒。デフォルトでは、１フレームの経過時間が渡されています。
		 *@return 移動後のキャラクターの座標。
		 */
		const CVector3& Execute(CVector3& moveSpeed, float deltaTime = 1.0f / GetStandardFrameRate());
		/*!
		 * @brief	座標を取得。
		 */
		const CVector3& GetPosition() const
		{
			return m_position;
		}
		/*!
		 * @brief	座標を設定。
		 */
		void SetPosition(const CVector3& pos)
		{
			m_position = pos;
		}

		/*!
		 * @brief	ジャンプ中か判定
		 */
		bool IsJump() const
		{
			return m_isJump;
		}
		/*!
		* @brief	地面上にいるか判定。
		*/
		bool IsOnGround() const
		{
			return m_isOnGround;
		}
		//壁に接触しているか？
		bool IsContactWall() const
		{
			return m_isContactWall;
		}
		//接触している壁の法線(平均)を取得
		const CVector3& GetContactWallNormal()const {
			return m_contactWallNormal;			
		}
		/*!
		* @brief	コライダーを取得。
		*/
		CapsuleCollider* GetCollider()
		{
			return &m_collider;
		}
		/*!
		* @brief	剛体を取得。
		*/
		RigidBody* GetRigidBody()
		{
			return &m_rigidBody;
		}
		/*!
		* @brief	剛体を物理エンジンから削除。。
		*/
		void RemoveRigidBoby();

		//壁から余分に押し出す距離を設定
		void SetOffset(float offset) {
			m_offsetOffset = offset;
		}

	private:
		bool				m_isInited = false;				//!<初期化済み？
		CVector3 			m_position = CVector3::Zero();	//!<座標。
		bool 				m_isJump = false;				//!<ジャンプ中？
		bool				m_isOnGround = true;			//!<地面の上にいる？
		bool				m_isContactWall = false;		//壁に接触してる？
		CVector3			m_contactWallNormal;			//接触している壁の法線(平均)
		CapsuleCollider		m_collider;						//!<コライダー。
		float				m_radius = 0.0f;				//!<カプセルコライダーの半径。
		float				m_height = 0.0f;				//!<カプセルコライダーの高さ。
		RigidBody			m_rigidBody;					//剛体。

		float m_offsetOffset = 1.0f;
	};
}