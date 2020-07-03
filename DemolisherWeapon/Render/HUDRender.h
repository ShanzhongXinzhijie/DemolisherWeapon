#pragma once
#include "IRander.h"

namespace DemolisherWeapon {
	class HUDRender :
		public IRander
	{
	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="HUDNum">���̃����_�[���S������HUD�ԍ�</param>
		/// <param name="screen_min">�`��͈�(�ŏ���)</param>
		/// <param name="screen_max">�`��͈�(�ő呤)</param>
		/// <param name="screenSize">�e�N�X�`���T�C�Y</param>
		void Init(int HUDNum, const CVector2& screen_min, const CVector2& screen_max, const CVector2& screenSize);
		
		/// <summary>
		/// �����_�[����
		/// </summary>
		void Render()override;
		void PostRender()override;

		/// <summary>
		/// ���T�C�Y
		/// </summary>
		/// <param name="screenSize">�e�N�X�`���T�C�Y</param>
		void Resize(const CVector2& screenSize);

	private:
		int m_HUDNum = 0;//���̃����_�[���S������HUD�ԍ�

		CVector2 m_textureSize;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_texture;	//�e�N�X�`��
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_RTV;		//�����_�[�^�[�Q�b�g�r���[
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;		//SRV

		//�o�b�N�o�b�t�@�ւ̕`��p
		Shader m_vs, m_ps;
		CPrimitive m_drawSpace;
		CVector2 m_screen_min, m_screen_max;
		CPrimitive::SVertex m_vertex[4];
		unsigned long m_index[4] = { 0,1,2,3 };
	};
}
