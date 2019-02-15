#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

class ConvertLinearToSRGBRender :
	public IRander
{
public:
	ConvertLinearToSRGBRender();
	~ConvertLinearToSRGBRender();

	void Init();
	void Release();

	void Render()override;

private:
	Shader m_vs, m_ps;
	ID3D11SamplerState* m_samplerState = nullptr;
};

}