#pragma once

#include "IRander.h"
#include "Graphic/Model/SkinModel.h"

namespace DemolisherWeapon {

class D3Render : public IRander
{
public:
	D3Render();
	~D3Render();

	void Render()override;
	void PostRender()override;

	void AddDrawModel(SkinModel* sm) {
		m_drawModelList.emplace_back(sm);
	};

private:
	std::list<SkinModel*> m_drawModelList;
};

}
