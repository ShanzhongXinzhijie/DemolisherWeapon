#pragma once

namespace DemolisherWeapon {

	//TODO
	//SkinModel��IDrawer�p��
	//SkinModelRender��LODDrawer�������A������

	/// <summary>
	/// �`�敨�̃C���^�[�t�F�[�X�N���X
	/// </summary>
	class IDrawer
	{
	public:
		IDrawer() = default;
		virtual ~IDrawer() {}

		/// <summary>
		/// �`��
		/// </summary>
		/// <param name="reverseCull">�ʂ𔽓]���邩</param>
		/// <param name="instanceNum">�`��C���X�^���X��</param>
		/// <param name="pBlendState">�u�����h�X�e�[�g</param>
		/// <param name="pDepthStencilState">�f�v�X�X�e���V���X�e�[�g</param>
		virtual void Draw(bool reverseCull = false,
			int instanceNum = 1,
			ID3D11BlendState* pBlendState = nullptr,
			ID3D11DepthStencilState* pDepthStencilState = nullptr) = 0;
	};

	/// <summary>
	/// Level of Detail �����s����Drawer
	/// </summary>
	class LODDrawer : public IDrawer {
	public:
		/// <summary>
		/// �`��
		/// </summary>
		void Draw(bool reverseCull = false,
			int instanceNum = 1,
			ID3D11BlendState* pBlendState = nullptr,
			ID3D11DepthStencilState* pDepthStencilState = nullptr)override {

			//��ʂɑ΂���傫������`�悷��h�����[�����߂�

		}

		/// <summary>
		/// �h�����[�̒ǉ�
		/// </summary>
		void AddDrawer(IDrawer* drawer, int lodLevel) {

		}

		/// <summary>
		/// �h�����[�̐؂�ւ��ݒ�
		/// </summary>

	private:
		std::unordered_map<int, IDrawer*> m_drawer;
	};
}