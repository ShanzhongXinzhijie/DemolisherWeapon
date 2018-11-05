#include "DWstdafx.h"
#include "MapChip.h"
#include "Level.h"

namespace DemolisherWeapon {

	MapChip::MapChip(const LevelObjectData& objData)
	{
		
		wchar_t filePath[256];
		swprintf_s(filePath, L"Assets/modelData/%s.cmo", objData.name);
		m_model.Init(filePath);
		m_model.SetPRS(objData.position, objData.rotation, objData.scale);
		//シャドウマップに書き込むか設定
		m_model.SetIsShadowCaster(objData.mapChipSetting.isShadowCaster);

		//静的物理オブジェクトをメッシュコライダーから作成する。
		if (objData.mapChipSetting.createPhysicsStaticObject) {
			m_physicsStaticObject = std::make_unique<PhysicsStaticObject>();
			m_physicsStaticObject->CreateMesh(m_model);
		}
	}

}