#include "DWstdafx.h"
#include "DirectXTKRender.h"

namespace DemolisherWeapon {

	void DirectXTKRender::Init(GameObjectManager* gom, CFpsCounter* fc) {
		m_gameObjectManager_Ptr = gom;
		m_fpscounter = fc;
	}

	void DirectXTKRender::Render() {
		//ここまでのコマンド実行
		//GetGraphicsEngine().ExecuteCommand();

		//ゲームオブジェクトによるポスト描画
		//(スプライトとかの描画)
		m_gameObjectManager_Ptr->PostRender();

		//FPS表示		
		m_fpscounter->Draw();

		//コマンド実行
		//GetGraphicsEngine().ExecuteCommandDirectXTK();
	}

}