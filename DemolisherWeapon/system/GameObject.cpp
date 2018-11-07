#include "DWstdafx.h"
#include "GameObject.h"

namespace DemolisherWeapon {

	IGameObject::IGameObject(bool isRegister) {
		if (isRegister) { AddGO(this); }
	}

	void GOStatusCaster::Cast(const GOStatus& status) {
		m_receiver->SetStatus(status);
	}

	void GameObjectManager::PostRender() {
		GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin();

		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetIsStart()) {
				go.gameObject->PostRender();
			}
		}

		GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();
	}
}