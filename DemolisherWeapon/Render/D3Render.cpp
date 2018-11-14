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
	for (auto& model : m_drawModelList) {
		model->Draw();
	}	
	//���X�^���C�U�[�X�e�[�g�߂�
	GetEngine().GetGraphicsEngine().ResetRasterizerState();
}
void D3Render::PostRender() {
	m_drawModelList.clear();
}

}