#include "DWstdafx.h"
#include "GameObject.h"

namespace DemolisherWeapon {

	IGameObject::IGameObject(bool isRegister) {
		if (isRegister) { AddGO(this); }
	}
	/*IGameObject::IGameObject(const IGameObject& go) {//�R�s�[�R���X�g���N�^
		if (go.IsRegistered()) { AddGO(this); }
	}*/
	/*{
		bool m_isStart = false;
		GORegister* m_register = nullptr;//�}�l�[�W���[�ɓo�^����Ă��邩(�|�C���^)
		GOStatus m_status;//���
		std::list<GOStatusCaster> m_statusCaster;//��Ԃ𑗐M����
	}*/

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