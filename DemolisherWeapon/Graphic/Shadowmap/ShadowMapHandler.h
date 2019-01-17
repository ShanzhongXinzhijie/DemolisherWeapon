#pragma once

namespace DemolisherWeapon {

	class ShadowMapHandler
	{
	public:
		ShadowMapHandler();
		~ShadowMapHandler();

		void ShadowMapUpdate();

		//������
		//UINT width, UINT height ...�V���h�E�}�b�v�̉𑜓x
		void Init(UINT width, UINT height, const CVector3& direction, const CVector3& target = CVector3::Zero(), const CVector3& area = { 100.0f, 100.0f, 100.0f }, float Near = 0.0f, const CVector3& dir_y = CVector3::Zero());
		void Release();

		//�V���h�E�}�b�v�͈̔͂̒��S�ʒu��ݒ�
		void SetTarget(const CVector3& target) {
			m_target = target;
			ShadowMapUpdate();
		}
		//���C�g�̕�����ݒ�
		void SetDirection(const CVector3& direction) {
			m_direction = direction; m_direction.Normalize();
			ShadowMapUpdate();
		}
		void SetNear(float Near) {
			m_near = Near;
			ShadowMapUpdate();
		}
		//�V���h�E�}�b�v�͈̔͂�ݒ�(Y��������AZ�����C�g�̕���)
		void SetArea(const CVector3& area) {
			m_area = area;
			ShadowMapUpdate();
		}
		//�������ݒ�
		void SetY(const CVector3& dir_y) {
			m_dirY = dir_y;	m_dirY.Normalize();
			ShadowMapUpdate();
		}

		//�[�x�o�C�A�X��ݒ�
		void SetDepthBias(float bias) {
			m_shadowMap->SetDepthBias(bias);
		}

	private:
		CShadowMap* m_shadowMap = nullptr;
		CVector3 m_direction, m_target, m_area, m_dirY;
		float m_near = 0.0f;
	};

}