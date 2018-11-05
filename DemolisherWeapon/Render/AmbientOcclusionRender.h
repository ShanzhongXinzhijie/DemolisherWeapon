#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

class AmbientOcclusionRender : public IRander
{
public:
	AmbientOcclusionRender();
	~AmbientOcclusionRender();

	void Init(float texScale = 0.5f);
	void Release();

	void Render()override;

	ID3D11ShaderResourceView*& GetAmbientOcclusionSRV() {
		return m_ambientOcclusionSRV;
	}

	//�L�������̐ݒ�
	void SetEnable(bool enable) { m_enable = enable; }
	bool GetEnable()const { return m_enable; }

private:
	bool m_enable = true;

	UINT m_textureSizeX=0, m_textureSizeY=0;

	Shader m_cs;
	ID3D11Texture2D* m_ambientOcclusionTex = nullptr;
	ID3D11ShaderResourceView* m_ambientOcclusionSRV = nullptr;
	ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
	ID3D11Buffer* m_cb = nullptr;

	//�萔�o�b�t�@
	struct SCSConstantBuffer {
		//�t���[���o�b�t�@�𑜓x
		unsigned int win_x;
		unsigned int win_y;

		//AO�o�̓e�N�X�`���𑜓x
		unsigned int ao_x;
		unsigned int ao_y;		

		//�r���[�v���W�F�N�V�����t�s��
		CMatrix mViewProjInv;

		//�����X�P�[��
		float distanceScale;
	};
};

}