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

		enum enBlendMode {
			enAlpha,enAdd,
		};

		/// <summary>
		/// �`�悷�郂�f����ǉ�
		/// </summary>
		/// <param name="sm">�`�悷�郂�f��</param>
		/// <param name="priority">�`�揇(���l���傫���قǌ�)(0�`DRAW_PRIORITY_MAX - 1)</param>
		/// <param name="blendmode">�u�����h���[�h</param>
		void AddDrawModel(SkinModel* sm, int priority, enBlendMode blendmode = enAlpha, bool reverse = false) {
			if (blendmode == enAdd) {
				m_drawModelList_Add[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back( sm,reverse );
			}
			if (blendmode == enAlpha) {
				m_drawModelList_Alpha[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back( sm,reverse );
			}
		};

	private:
		//�`�悷�郂�f���̃��X�g
		std::list<std::pair<SkinModel*, bool>> m_drawModelList_Alpha[DRAW_PRIORITY_MAX], m_drawModelList_Add[DRAW_PRIORITY_MAX];
		//�u�����h�X�e�[�g
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaBlendState, m_addBlendState;
		//�f�v�X�X�e���V���X�e�[�g
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	};

}