#pragma once
#include "IRander.h"

namespace DemolisherWeapon {

	class PostDrawModelRender :
		public IRander
	{
	public:
		PostDrawModelRender() = default;

		void Init();

		void Render()override;
		void PostRender()override;

		/// <summary>
		/// �`�悷�郂�f����ǉ�
		/// </summary>
		/// <param name="sm">�`�悷�郂�f��</param>
		/// <param name="priority">�`�揇(���l���傫���قǌ�)(0�`DRAW_PRIORITY_MAX - 1)</param>
		/// <param name="isAdd">���Z�u�����h���H</param>
		void AddDrawModel(SkinModel* sm, int priority, bool isAdd = false) {
			if (isAdd) {
				m_drawModelList_Add.emplace_back(sm);
			}
			else {
				m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back(sm);
			}
		};

	private:
		//�`�悷�郂�f���̃��X�g
		std::list<SkinModel*> m_drawModelList[DRAW_PRIORITY_MAX], m_drawModelList_Add;
		//���Z�u�����h�X�e�[�g
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_addBlendState;
	};

}