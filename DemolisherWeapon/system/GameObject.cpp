#include "DWstdafx.h"
#include "GameObject.h"

namespace DemolisherWeapon {

	GOStatusCaster::~GOStatusCaster() {
		if (m_alive && m_receiver) {
			m_receiver->SetCaster(nullptr);
		}
	}
	GODeathListenerRegister::~GODeathListenerRegister() {
		if (enable && listener) {
			listener->SetResister(nullptr);
		}
	}

	IGameObject::IGameObject(bool isRegister, bool quickStart) {
		if (isRegister) { AddGO(this); }
		m_quickStart = quickStart;
	}

	//名前をつける
	void IGameObject::SetName(const wchar_t* objectName) {
		SetNameGO(this, objectName);
	}

	void GOStatusCaster::Cast(const GOStatus& status) {
		m_receiver->SetStatus(status);
	}

	void GameObjectManager::PostRender() {
		GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin(DirectX::SpriteSortMode_BackToFront);
		DirectX::CommonStates state(GetEngine().GetGraphicsEngine().GetD3DDevice());
		GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode_BackToFront, state.NonPremultiplied());

		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PostRender();
			}
		}

		GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();
		GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->End();
		GetEngine().GetGraphicsEngine().ResetLayerDepthCnt();
	}
}