#pragma once

namespace DemolisherWeapon {
	namespace GameObj {
		namespace Suicider {

			template <class X>
			class CParticle :
				public IQSGameObject
			{
			public:
				void Update()override {
					//���t���[���̕ω�
					if (m_graphic) {
						m_graphic->SetPos(m_graphic->GetPos() + m_move);
						m_graphic->SetScale(m_graphic->GetScale()*m_scaling);
						m_graphic->SetRot(m_rotation * m_graphic->GetRot());
					}
					//����
					if (m_lifeTime < 0) { return; }
					m_lifeTime--; if (m_lifeTime <= 0) { delete this; return; }
				}

			public:
				/// <summary>
				/// �R���X�g���N�^
				/// </summary>
				CParticle() = default;
				/// <param name="graphic">�p�[�e�B�N���̌�����</param>
				/// <param name="lifetime">����(lifetime��0�Ŏ����Ȃ�)</param>
				CParticle(std::unique_ptr<X>&& graphic, int lifetime = -1) {
					Play(std::move(graphic), lifetime);
				}

				/// <summary>
				/// �Đ�
				/// </summary>
				/// <param name="graphic">�p�[�e�B�N���̌�����</param>
				/// <param name="lifetime">����(lifetime��0�Ŏ����Ȃ�)</param>
				void Play(std::unique_ptr<X>&& graphic, int lifetime = -1) {
					m_graphic.reset();
					m_graphic = std::move(graphic); m_lifeTime = lifetime;
				}

				//���t���[���̈ړ��ʂ�ݒ�
				void SetMove(const CVector3& vec) { m_move = vec; }
				//���t���[���̊g�嗦��ݒ�
				void SetScaling(const CVector3& scale) { m_scaling = scale; }
				//���t���[���̉�]�ʂ�ݒ�
				void SetRotation(const CQuaternion& rot) { m_rotation = rot; }

				//�p�[�e�B�N���̌����ڂ��擾
				X* GetGraphic() { return m_graphic.get(); }

				//�c��������擾
				int GetLifeTime()const { return m_lifeTime; }

			private:
				std::unique_ptr<X> m_graphic;
				int m_lifeTime = -1;
				CVector3 m_move, m_scaling = CVector3::One();
				CQuaternion m_rotation;
			};

		}
	}
}
