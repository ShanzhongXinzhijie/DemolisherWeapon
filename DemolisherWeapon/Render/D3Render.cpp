#include "DWstdafx.h"
#include "D3Render.h"

namespace DemolisherWeapon {

D3Render::D3Render()
{
}


D3Render::~D3Render()
{
}

void D3Render::Render() {
	//GPUイベントの開始
	GetGraphicsEngine().BeginGPUEvent(L"D3Render");

	for (auto& model : m_drawModelList) {
		model->Draw();
	}	
	//ラスタライザーステート戻す
	GetEngine().GetGraphicsEngine().ResetRasterizerState();

	//GPUイベントの終了
	GetGraphicsEngine().EndGPUEvent();
}
void D3Render::PostRender() {
	m_drawModelList.clear();
}

}