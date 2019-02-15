#pragma once
#include "IRander.h"
#include "Graphic/Shader/Shader.h"
#include "ShadowMapRender.h"

namespace DemolisherWeapon {

class DefferdRender :
	public IRander
{
public:
	DefferdRender();
	~DefferdRender();

	void Init();
	void Release();

	void Render()override;

private:
	Shader m_vs;
	Shader m_ps;
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11SamplerState* m_samplerStateNoFillter = nullptr;

	//シャドウマップ用の定数バッファ
	struct ShadowConstantBuffer {
		CMatrix mViewProjInv;
		CMatrix mLVP[ShadowMapRender::SHADOWMAP_NUM];
		CVector4 shadowDir[ShadowMapRender::SHADOWMAP_NUM];
		CVector4 enableShadowMap[ShadowMapRender::SHADOWMAP_NUM];
		CVector4 cascadeArea[ShadowMapRender::SHADOWMAP_NUM];

		int boolAO;
		//定数バッファは float4(4*4=16)で区切られる
	};

	ID3D11Buffer* m_scb = nullptr;	
	ID3D11SamplerState* m_samplerComparisonState = nullptr;

};

}
