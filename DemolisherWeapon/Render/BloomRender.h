#pragma once
#include "IRander.h"
#include "GaussianBlurRender.h"

namespace DemolisherWeapon {
	class BloomRender : public IRander
	{
	public:
		BloomRender() = default;
		~BloomRender() { Release(); }

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="texScale">texScale*3D�t���[���o�b�t�@�T�C�Y = �e�N�X�`���T�C�Y</param>
		void Init(float texScale = 0.5f);
		void Release();

		void Render() override;

		//�V�F�[�_�[���\�[�X�r���[�̎擾
		ID3D11ShaderResourceView*& GetSRV() {
			return m_SRV;
		}

		//�L�������̐ݒ�
		void SetEnable(bool enable) { m_enable = enable; }
		bool GetEnable()const { return m_enable; }

		//�P�x�������l�̐ݒ�
		void SetLuminanceThreshold(float f) { m_luminanceThreshold = f; }
		float GetLuminanceThreshold()const { return m_luminanceThreshold; }

	private:
		bool m_enable = true;

		Shader m_cs;
		ID3D11Texture2D* m_tex = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView*	m_outputUAV = nullptr;
		ID3D11RenderTargetView* m_RTV = nullptr;
		UINT m_textureSizeX = 0, m_textureSizeY = 0;

		//�萔�o�b�t�@
		//[Bloom.fx:CSCb]
		struct SCSConstantBuffer {
			//�t���[���o�b�t�@�𑜓x
			unsigned int win_x;
			unsigned int win_y;
			//�o�̓e�N�X�`���𑜓x
			unsigned int out_x;
			unsigned int out_y;

			//�P�x�������l
			float luminanceThreshold;
		};
		ID3D11Buffer* m_cb = nullptr;//�P�x�������l

		//�ݒ�
		float m_luminanceThreshold = 1.0f;

		//���ȃe�N�X�`��
		ID3D11ShaderResourceView* m_interferenceFringesSRV = nullptr;

		//�K�E�X�u���[�����_�[
		GaussianBlurRender m_gaussBlur;

		//�����p
		ID3D11BlendState* m_blendState = nullptr;
		Shader m_vs, m_ps;
		ID3D11SamplerState* m_samplerState = nullptr;
	};
}
