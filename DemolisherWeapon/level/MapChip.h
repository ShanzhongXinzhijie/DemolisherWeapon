#pragma once

#include "physics/PhysicsStaticObject.h"

namespace DemolisherWeapon {

	struct LevelObjectData;

	struct MapChipSetting {
		bool isShadowCaster = true;//シャドウマップに書き込むか
		bool createPhysicsStaticObject = true;//静的物理オブジェクトを作成するか
	};

	/*!
	* @brief	マップチップ。
	*/
	class MapChip {
	public:
		/*!
		* @brief	コンストラクタ。
		*@param[in] objData			オブジェクト情報。
		*/
		MapChip(const LevelObjectData& objData);

		/*!
		*@brief	スキンモデルレンダラーを取得。
		*/
		const GameObj::CSkinModelRender& GetSkinModelRender()
		{
			return m_model;
		}
	private:
		GameObj::CSkinModelRender m_model;			//!<モデル。
		std::unique_ptr<PhysicsStaticObject> m_physicsStaticObject;	//!<静的物理オブジェクト。		
	};

}