#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class DepthOfFieldRender :
		public IRander
	{
	private:
		//�萔�o�b�t�@
		//[DOF.fx:PSCb]
		struct SPSConstantBuffer {
			float focus = 0.125f;
			float focusArea = 0.1f;
			float Near = 0.0f;
			float Far = 0.5f;
			
			//�J�����ݒ�
			float camNear = 1.0f;
			float camFar = 100.0f;
		};

	public:
		DepthOfFieldRender();
		~DepthOfFieldRender();

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="texScale">texScale*3D�t���[���o�b�t�@�T�C�Y = �k���e�N�X�`���T�C�Y</param>
		void Init(float texScale = 0.5f);
		void Release();

		void Render() override;

		//�L�������̐ݒ�
		void SetEnable(bool enable) { m_enable = enable; }
		bool GetEnable()const { return m_enable; }

		//�œ_�����̐ݒ�
		void SetFocus(float focus) { m_cbSetting.focus = focus; }
		void SetFocusArea(float area) { m_cbSetting.focusArea = area; }//�œ_�̍����͈�
		void SetNear(float Near) { m_cbSetting.Near = Near; }
		void SetFar(float Far) { m_cbSetting.Far = Far; }

	private:
		bool m_enable = true;

		//�T���v���[
		ID3D11SamplerState* m_samplerState = nullptr;

		//�k���e�N�X�`��
		ID3D11Texture2D* m_tex = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11RenderTargetView* m_RTV = nullptr;
		UINT m_textureSizeX = 0, m_textureSizeY = 0;
		Shader m_vsResize, m_psResize;
		D3D11_VIEWPORT m_viewport;//�r���[�|�[�g

		//�K�E�X�u���[�����_�[
		GaussianBlurRender m_gaussBlur;

		//�����V�F�[�_
		Shader m_vsCompo, m_psCompo;

		//�萔�o�b�t�@
		SPSConstantBuffer m_cbSetting;
		ID3D11Buffer* m_cb = nullptr;
	};

}