#include "DWstdafx.h"
#include "GameObject.h"

namespace DemolisherWeapon {

	bool GORegister::GetIsStart() {
		if (!isStartedGO && m_isQuickStartGO) {
			return gameObject->GetIsStart();
		}
		return isStartedGO;
	}

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
		if (IsRegistered()) { m_register->isNoPendingkill = false; }

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

	void GameObjectManager::Start() {
		for (auto& go : m_runFuncGOList[IGameObject::enStart]) {
			if (go->isNoPendingkill && go->isEnableGO && !go->GetIsStart()) {
				if (go->gameObject->Start()) {
					go->gameObject->SetIsStart();
				}
			}
		}
	}
	void GameObjectManager::PreLoopUpdate() {
		for (auto& go : m_runFuncGOList[IGameObject::enPreLoopUpdate]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->PreLoopUpdate();
			}
		}
	}
	void GameObjectManager::Update() {
		for (auto& go : m_runFuncGOList[IGameObject::enPreUpdate]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->PreUpdate();
			}
		}
		for (auto& go : m_runFuncGOList[IGameObject::enUpdate]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->Update();
			}
		}
		for (auto& go : m_runFuncGOList[IGameObject::enPostUpdate]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->PostUpdate();
			}
		}
	}
	void GameObjectManager::PostLoopUpdate() {
		for (auto& go : m_runFuncGOList[IGameObject::enPostLoopUpdate]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->PostLoopUpdate();
			}
		}
		for (auto& go : m_runFuncGOList[IGameObject::enPostLoopPostUpdate]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->PostLoopPostUpdate();
			}
		}
	}
	void GameObjectManager::Pre3DRender(int num) {
		for (auto& go : m_runFuncGOList[IGameObject::enPre3DRender]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->Pre3DRender(num);
			}
		}
	}
	void GameObjectManager::HUDRender(int HUDNum) {
#ifndef DW_DX12_TEMPORARY

		GetGraphicsEngine().GetSpriteBatch()->Begin(DirectX::SpriteSortMode_BackToFront);
		GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode_BackToFront, GetGraphicsEngine().GetCommonStates().NonPremultiplied());

		for (auto& go : m_runFuncGOList[IGameObject::enHUDRender]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->HUDRender(HUDNum);
			}
		}

		GetGraphicsEngine().GetSpriteBatch()->End();
		GetGraphicsEngine().GetSpriteBatchPMA()->End();
		//GetEngine().GetGraphicsEngine().ResetLayerDepthCnt();

#endif
	}
	void GameObjectManager::PostRender() {
#ifdef DW_DX11
		GetEngine().GetGraphicsEngine().GetSpriteBatch()->Begin(DirectX::SpriteSortMode_BackToFront);
		GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->Begin(DirectX::SpriteSortMode_BackToFront, GetGraphicsEngine().GetCommonStates().NonPremultiplied());
#endif
#ifdef DW_DX12
		ID3D12DescriptorHeap* heaps[] = { GetGraphicsEngine().GetDirectXTK12DescriptorHeap() };
		GetGraphicsEngine().GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
		GetGraphicsEngine().GetSpriteBatch()->Begin(GetGraphicsEngine().GetCommandList(), DirectX::SpriteSortMode::SpriteSortMode_BackToFront);
		GetGraphicsEngine().GetSpriteBatchPMA()->Begin(GetGraphicsEngine().GetCommandList(), DirectX::SpriteSortMode::SpriteSortMode_BackToFront);
#endif

		for (auto& go : m_runFuncGOList[IGameObject::enPostRender]) {
			if (go->isNoPendingkill && go->isEnableGO && go->GetIsStart()) {
				go->gameObject->PostRender();
			}
		}

		GetEngine().GetGraphicsEngine().GetSpriteBatch()->End();
		GetEngine().GetGraphicsEngine().GetSpriteBatchPMA()->End();
		GetEngine().GetGraphicsEngine().ResetLayerDepthCnt();
	}
	void GameObjectManager::Hell() {
		//m_gameObjectMap�̍폜
		if (m_isDeleteGOThisFrame) {
			auto it = m_gameObjectMap.begin();
			auto endit = m_gameObjectMap.end();
			while (it != endit) {
				if (!(*it).second->isNoPendingkill) {
					it = m_gameObjectMap.erase(it);//�폜
				}
				else {
					++it;
				}
			}
		}

		//TODO �S�I�[�o�[���C�h�̂��
		//master 50000��30
		//GO2 50000��10

		//�֐����s���X�g����Q�[���I�u�W�F�N�g�Q�Ƃ��폜
		DeleteFromFuncList();

		//m_gameObjectList�̍폜
		if (m_isDeleteGOThisFrame) {
			bool isRun = false;
			auto it = m_gameObjectList.begin();
			auto endit = m_gameObjectList.end();
			while (it != endit) {
				if (!(*it).isNoPendingkill) {
					isRun = true;
					if ((*it).nowOnHell) {//���ڂō폜
						it = m_gameObjectList.erase(it);//�폜
					}
					else {
						(*it).nowOnHell = true;
						++it;
					}
				}
				else {
					++it;
				}
			}
			if (!isRun) {
				//�폜�������s�Ȃ����OFF
				m_isDeleteGOThisFrame = false;
			}
		}
	}
	void GameObjectManager::DeleteFromFuncList() {
		int funcType = 0;
		for (auto& list : m_runFuncGOList) {
			if (m_isDeleteGOThisFrame || m_isCheckVFuncThisFrame[funcType]) {
				auto it = list.begin();
				auto endit = list.end();
				while (it != endit) {
					if (!(*it)->isNoPendingkill || !(*it)->gameObject->GetIsOverrideVFunc(static_cast<IGameObject::VirtualFuncs>(funcType))) {
						it = list.erase(it);//�폜
					}
					else {
						++it;
					}
				}
			}
			m_isCheckVFuncThisFrame[funcType] = false;
			funcType++;//���̊֐���
		}
	}
}