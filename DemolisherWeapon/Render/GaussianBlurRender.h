#pragma once

namespace DemolisherWeapon {

	class GaussianBlurRender{
	public:
		GaussianBlurRender();
		~GaussianBlurRender();

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="souce">�ڂ����V�F�[�_�[���\�[�X�r���[</param>
		/// <param name="dispersion">�ڂ����̋���</param>
		void Init(ID3D11ShaderResourceView*& souce, float dispersion);

		void Release();

		/// <summary>
		/// �K�E�X�u���[��������
		/// </summary>
		void Blur();

		/// <summary>
		/// �V�F�[�_�[���\�[�X�r���[�̎擾
		/// </summary>
		/// <returns></returns>
		ID3D11ShaderResourceView*& GetSRV() {
			return m_outputYSRV;
		}

	private:
		Shader m_vsx, m_vsy, m_ps;

		//���̓e�N�X�`��
		ID3D11ShaderResourceView* m_souce = nullptr;
		//�o�̓e�N�X�`��
		ID3D11Texture2D*			m_outputX = nullptr;
		ID3D11RenderTargetView*		m_outputXRTV = nullptr;
		ID3D11ShaderResourceView*	m_outputXSRV = nullptr;

		ID3D11Texture2D*			m_outputY = nullptr;
		ID3D11RenderTargetView*		m_outputYRTV = nullptr;
		ID3D11ShaderResourceView*	m_outputYSRV = nullptr;

		D3D11_TEXTURE2D_DESC m_texDesc;

		//�萔�o�b�t�@
		static const int NUM_WEIGHTS = 8; 
		struct SBlurParam {
			CVector4 offset;
			float weights[NUM_WEIGHTS];
		};
		SBlurParam m_blurParam;
		ID3D11Buffer* m_cb = nullptr;

		//�T���v���[
		ID3D11SamplerState* m_samplerState = nullptr;

		D3D11_VIEWPORT m_viewport;//�r���[�|�[�g
	};

}
