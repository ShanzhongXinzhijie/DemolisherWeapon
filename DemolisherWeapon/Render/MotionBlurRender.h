#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

#include"../Preset/shader/MotionBlurHeader.h"

class MotionBlurRender :
	public IRander
{
public:
	static constexpr float DEFAULT_MBLUR_SCALE = 0.15f;

public:
	MotionBlurRender();
	~MotionBlurRender();

	void Init();
	void Release();

	//IRanderの実装
	void Resize()override;
	void Render()override;

	//これが有効か設定
	void SetEnable(bool enable) {
		m_isEnable = enable;
	}

	//モーションブラースケールを設定
	void SetMotionBlurScale(float scale = DEFAULT_MBLUR_SCALE) {
		m_motionBlurScale = scale;
	}
	float GetMotionBlurScale()const {
		return m_motionBlurScale;
	}

private:
	void PSBlur(ID3D11DeviceContext* rc);
	void CSBlur(ID3D11DeviceContext* rc);

private:
	bool m_isEnable = true;

	Shader m_cs, m_vs, m_ps;
	ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
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
		float distanceScale;
	};

	ID3D11SamplerState* m_samplerState = nullptr;

	//モーションブラースケール
	float m_motionBlurScale = DEFAULT_MBLUR_SCALE;
};

}
