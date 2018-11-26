#include "DWstdafx.h"
#include "GameObject.h"

namespace DemolisherWeapon {

	IGameObject::IGameObject(bool isRegister) {
		if (isRegister) { AddGO(this); }
	}
	/*IGameObject::IGameObject(const IGameObject& go) {//コピーコンストラクタ
		if (go.IsRegistered()) { AddGO(this); }
	}*/
	/*{
		bool m_isStart = false;
		GORegister* m_register = nullptr;//マネージャーに登録されているか(ポインタ)
		GOStatus m_status;//状態
		std::list<GOStatusCaster> m_statusCaster;//状態を送信する
	}*/

	//名前をつける
	void IGameObject::SetName(const wchar_t* objectName) {
		SetNameGO(this, objectName);
	}

	void GOStatusCaster::Cast(const GOStatus& status) {
		m_receiver->SetStatus(status);
	}

	void GameObjectManager::PostRender() {
		GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin();

		for (auto& go : m_gameObjectList) {
			if (go.isEnable && go.gameObject->GetEnable() && go.gameObject->GetIsStart()) {
				go.gameObject->PostRender();
			}
		}

		GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();
	}
}