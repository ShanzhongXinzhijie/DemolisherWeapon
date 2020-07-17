#include "DWstdafx.h"
#include "DirectXTKRender.h"

namespace DemolisherWeapon {

	void DirectXTKRender::Init(GameObjectManager* gom, CFpsCounter* fc) {
		m_gameObjectManager_Ptr = gom;
		m_fpscounter = fc;
	}

	void DirectXTKRender::Render() {
		//�����܂ł̃R�}���h���s
		//GetGraphicsEngine().ExecuteCommand();

		//�Q�[���I�u�W�F�N�g�ɂ��|�X�g�`��
		//(�X�v���C�g�Ƃ��̕`��)
		m_gameObjectManager_Ptr->PostRender();

		//FPS�\��		
		m_fpscounter->Draw();

		//�R�}���h���s
		//GetGraphicsEngine().ExecuteCommandDirectXTK();
	}

}