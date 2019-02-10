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

	//�萔�o�b�t�@
	struct SCSConstantBuffer {
		//�t���[���o�b�t�@�𑜓x
		unsigned int win_x;
		unsigned int win_y;
		//�����X�P�[��
		float distanceScale;
	};
	struct SPSConstantBuffer {
		float DistantThreshold;
	};

	ID3D11SamplerState* m_samplerState = nullptr;
};

}
