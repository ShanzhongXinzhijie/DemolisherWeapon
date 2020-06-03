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
		for (auto& b : m_isRunFunc) {
			b = true;
		}
		if (isRegister) { AddGO(this); }
		m_quickStart = quickStart;
	}
	IGameObject::~IGameObject() {
		//�}�l�[�W���[�ɃQ�[���I�u�W�F�N�g�̍폜���s��ꂽ���Ƃ�ʒm
		GetEngine().GetGameObjectManager().EnableIsDeleteGOThisFrame();

		//�L���łȂ��񂾁I
		if (IsRegistered()) { m_register->isEnable = false; }

		//�X�e�[�^�X�X�V(���񂾂��I)
		m_status.m_isDead = true;
		CastStatus();

		//�f�X���X�i�[�ɒʒm
		GODeathListener::SDeathParam param;
		param.gameObject = this;
		auto it = m_deathListeners.begin();
		auto endit = m_deathListeners.end();
		while (it != endit) {
			if ((*it).enable) {
				(*it).listener->RunFunction(param);
				it++;
			}
			else {
				it = m_deathListeners.erase(it);//�폜
			}
		}
	}

	//���z�֐��̎��s����߂�
	void IGameObject::OffIsRunVFunc(VirtualFuncs type) {
		m_isRunFunc[type] = false;
		//�}�l�[�W���[�ɒʒm
		GetEngine().GetGameObjectManager().EnableIsCheckVFuncThisFrame(type);
	}

	//���O������
	void IGameObject::SetName(const wchar_t* objectName) {
		SetNameGO(this, objectName);
	}

	void GOStatusCaster::Cast(const GOStatus& status) {
		m_receiver->SetStatus(status);
	}

	void GameObjectManager::HUDRender(int HUDNum) {
		GetGraphicsEngine().GetSpriteBatch()->Begin(DirectX::SpriteSortMode_BackToFront);
		GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode_BackToFront, GetGraphicsEngine().GetCommonStates().NonPremultiplied());

		for (auto& go : m_runFuncGOList[IGameObject::enHUDRender]) {
			if (go->isEnable && go->gameObject->GetEnable() && go->gameObject->GetIsStart()) {
				go->gameObject->HUDRender(HUDNum);
			}
		}

		GetGraphicsEngine().GetSpriteBatch()->End();
		GetGraphicsEngine().GetSpriteBatchPMA()->End();
		//GetEngine().GetGraphicsEngine().ResetLayerDepthCnt();
	}
	void GameObjectManager::PostRender() {
		GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin(DirectX::SpriteSortMode_BackToFront);
		GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode_BackToFront, GetGraphicsEngine().GetCommonStates().NonPremultiplied());

		for (auto& go : m_runFuncGOList[IGameObject::enPostRender]) {
			if (go->isEnable && go->gameObject->GetEnable() && go->gameObject->GetIsStart()) {
				go->gameObject->PostRender();
			}
		}

		GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();
		GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->End();
		GetEngine().GetGraphicsEngine().ResetLayerDepthCnt();
	}
}