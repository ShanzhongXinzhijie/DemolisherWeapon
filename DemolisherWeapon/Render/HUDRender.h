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
		/// <param name="screen_min">�`��͈�(�ŏ���)</param>
		/// <param name="screen_max">�`��͈�(�ő呤)</param>
		/// <param name="screenSize">�e�N�X�`���T�C�Y</param>
		void Init(int HUDNum, const CVector2& screen_min, const CVector2& screen_max, const CVector2& screenSize);
		
		/// <summary>
		/// �����_�[����
		/// </summary>
		void Render();

	private:
		int m_HUDNum = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_texture;	//�e�N�X�`��
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_RTV;		//�����_�[�^�[�Q�b�g�r���[
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;		//SRV

		//�o�b�N�o�b�t�@�ւ̕`��p
		Shader m_vs, m_ps;
		CPrimitive m_drawSpace;
		CPrimitive::SVertex m_vertex[4];
		unsigned long m_index[4] = { 0,1,2,3 };
	};
}
