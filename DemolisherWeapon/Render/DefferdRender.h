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

	//���L���[�u�}�b�v���Z�b�g
	//��MipMap���K�v�ł�(���Ȃ��Ƃ�8������)
	void SetAmbientCubeMap(const wchar_t* filePass);
	void SetIsAmbientCubeMap(bool enable) {
		m_isAmbCube = enable;
	}

private:
	Shader m_vs;
	Shader m_ps;
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11SamplerState* m_samplerStateNoFillter = nullptr;

	//�V���h�E�}�b�v�p�̒萔�o�b�t�@
	struct ShadowConstantBuffer {
		CMatrix mViewProjInv;
		CMatrix mLVP[ShadowMapRender::SHADOWMAP_NUM];
		CVector4 shadowDir[ShadowMapRender::SHADOWMAP_NUM];
		CVector4 enableShadowMap[ShadowMapRender::SHADOWMAP_NUM];
		CVector4 cascadeArea[ShadowMapRender::SHADOWMAP_NUM];

		int boolAO;
		int boolAmbientCube;
		//�萔�o�b�t�@�� float4(4*4=16)�ŋ�؂���
	};

	ID3D11Buffer* m_scb = nullptr;	
	ID3D11SamplerState* m_samplerComparisonState = nullptr;

	bool m_isAmbCube = false;
	ID3D11ShaderResourceView* m_ambientCube = nullptr;
};

}
