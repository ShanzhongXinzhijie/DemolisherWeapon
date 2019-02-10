#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

#include"../Preset/shader/MotionBlurHeader.h"

class MotionBlurRender :
	public IRander
{
public:
	MotionBlurRender();
	~MotionBlurRender();

	void Init();
	void Release();

	void SetEnable(bool enable) {
		m_isEnable = enable;
	}

	void Render()override;

private:
	void PSBlur(ID3D11DeviceContext* rc);
	void CSBlur(ID3D11DeviceContext* rc);

private:
	bool m_isEnable = true;

	Shader m_cs, m_vs, m_ps;
	ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
	ID3D11UnorderedAccessView*	m_maskUAV = nullptr;
	ID3D11Buffer* m_cb = nullptr;
	ID3D11Buffer* m_cbPS = nullptr;

	//定数バッファ
	struct SCSConstantBuffer {
		//フレームバッファ解像度
		unsigned int win_x;
		unsigned int win_y;
		//距離スケール
		float distanceScale;
	};
	struct SPSConstantBuffer {
		float DistantThreshold;
	};

	ID3D11SamplerState* m_samplerState = nullptr;
};

}
