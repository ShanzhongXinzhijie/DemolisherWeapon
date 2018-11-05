#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

class MotionBlurRender :
	public IRander
{
public:
	MotionBlurRender();
	~MotionBlurRender();

	void Init();
	void Release();

	void Render()override;

	void PSBlur(ID3D11DeviceContext* rc);
	void CSBlur(ID3D11DeviceContext* rc);

private:
	Shader m_cs, m_vs, m_ps;
	ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
	ID3D11UnorderedAccessView*	m_maskUAV = nullptr;
	ID3D11Buffer* m_cb = nullptr;

	//定数バッファ
	struct SCSConstantBuffer {
		//フレームバッファ解像度
		unsigned int win_x;
		unsigned int win_y;
		//距離スケール
		float distanceScale;
	};

	ID3D11SamplerState* m_samplerState = nullptr;
};

}
