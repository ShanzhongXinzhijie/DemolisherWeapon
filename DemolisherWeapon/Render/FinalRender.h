#pragma once

#include"Graphic/CPrimitive.h"

namespace DemolisherWeapon {

class CFinalRenderTarget {
public:
	~CFinalRenderTarget();

	void Init();
	void Release();

	ID3D11Texture2D*			GetTex(int num = -1) { if (num == 0 || num == 1) { return m_Tex[num]; } else { return m_Tex[m_now]; } }
	ID3D11RenderTargetView*&	GetRTV(int num = -1) { if (num == 0 || num == 1) { return m_RTV[num]; } else { return m_RTV[m_now]; } }
	ID3D11ShaderResourceView*&	GetSRV(int num = -1) { if (num == 0 || num == 1) { return m_SRV[num]; } else { return m_SRV[m_now]; } }

	ID3D11DepthStencilView* GetDSV() { return m_depthStencilView; }

	//�g�p�Ԃ̓���ւ�
	void Swap() {
		m_now ^= 1;
	}
	void SetNow(int n) {
		m_now = n;
	}
	
	void Copy();

	void Clear(int num = -1);
	void AllClear();

private:
	ID3D11Texture2D*			m_Tex[2] = { nullptr };//�e�N�X�`��
	ID3D11RenderTargetView*		m_RTV[2] = { nullptr };//RTV
	ID3D11ShaderResourceView*	m_SRV[2] = { nullptr };//SRV

	ID3D11Texture2D*		m_depthStencilTex  = nullptr;//�f�v�X�X�e���V���e�N�X�`��
	ID3D11DepthStencilView* m_depthStencilView = nullptr;//�f�v�X�X�e���V���r���[

	int m_now = 0;
};

class FinalRender : public IRander
{
public:
	~FinalRender();

	void Init(const CVector2 screen_min = { 0.0f,0.0f }, const CVector2 screen_max = { 1.0f,1.0f }, bool isLinearSample = true);
	void Release();

	void Render()override;

	//void SetFinalRenderTarget();

	//CFinalRenderTarget& GetFRT() { return m_FRT; }

	//�c�Ȏ����̌W�����v�Z
	static float Calc_k4(float fov);
	//�c�Ȏ�����������2D���W���擾
	static CVector2 CalcLensDistortion(const CVector2& pos, GameObj::ICamera* cam);

	//�c�Ȏ����̗L���E������ݒ�
	static void SetIsLensDistortion(bool enable) {
		m_isLensDistortion = enable;
	}
	//�c�Ȏ����̓x������ݒ�
	static void SetLensDistortionScale(float scale) {
		m_lensDistortionScale = scale;
	}
	//�A���`�G�C���A�X�̗L���E������ݒ�
	static void SetIsAntiAliasing(bool enable) {
		m_isAntiAliasing = enable;
	}

private:
	//�V�F�[�_�[�}�N��
	enum ShaderTypeMask {
		enOFF = 0b00,			//�S�Ė���
		enLensDistortion = 0b01,
		enAntialiasing = 0b10,
		enALL = 0b11,			//�S�ėL��
		enNum,					//�S�p�^�[���̐�
	};
	//�}�N���̐�
	static constexpr int MACRO_NUM = 2;

	Shader m_vs;
	Shader m_ps[ShaderTypeMask::enNum];
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11Buffer* m_cb = nullptr;

	struct SPSConstantBuffer {
		float k4;
		float LENS_DISTORTION_UV_MAGNIFICATION;
		float ASPECT_RATIO;
		float INV_ASPECT_RATIO;

		float resolution[2];
	};

	CPrimitive m_drawSpace;
	CPrimitive::SVertex m_vertex[4];
	int m_index[4] = { 0,1,2,3 };

	ID3D11ShaderResourceView* m_gridTex = nullptr;

	//�s�N�Z���V�F�[�_�ݒ�
	static bool m_isLensDistortion;
	static float m_lensDistortionScale;
	static bool m_isAntiAliasing;
};

}