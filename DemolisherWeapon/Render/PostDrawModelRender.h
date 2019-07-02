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

		//�`�悷�郂�f����ǉ�
		void AddDrawModel(SkinModel* sm, int priority) {
			m_drawModelList[CMath::Clamp(priority, 0, DRAW_PRIORITY_MAX - 1)].emplace_back(sm);
		};

	private:
		//�`�悷�郂�f���̃��X�g
		std::list<SkinModel*> m_drawModelList[DRAW_PRIORITY_MAX];
		//���Z�u�����h�X�e�[�g
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_addBlendState;
	};

}