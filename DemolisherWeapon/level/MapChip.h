#pragma once

#include "physics/PhysicsStaticObject.h"

namespace DemolisherWeapon {

	struct LevelObjectData;

	struct MapChipSetting {
		bool isShadowCaster = true;//�V���h�E�}�b�v�ɏ������ނ�
		bool createPhysicsStaticObject = true;//�ÓI�����I�u�W�F�N�g���쐬���邩
	};

	/*!
	* @brief	�}�b�v�`�b�v�B
	*/
	class MapChip {
	public:
		/*!
		* @brief	�R���X�g���N�^�B
		*@param[in] objData			�I�u�W�F�N�g���B
		*/
		MapChip(const LevelObjectData& objData);

		/*!
		*@brief	�X�L�����f�������_���[���擾�B
		*/
		const GameObj::CSkinModelRender& GetSkinModelRender()
		{
			return m_model;
		}
	private:
		GameObj::CSkinModelRender m_model;			//!<���f���B
		std::unique_ptr<PhysicsStaticObject> m_physicsStaticObject;	//!<�ÓI�����I�u�W�F�N�g�B		
	};

}