#pragma once

namespace DemolisherWeapon {
	namespace GameObj {
		namespace Suicider {

			class CDeleteTimer :
				public IGameObject
			{
			public:
				/// <summary>
				/// �R���X�g���N�^
				/// </summary>
				CDeleteTimer() = default;
				/// <param name="target">�폜����Q�[���I�u�W�F�N�g</param>
				/// <param name="timer">�폜�܂ł̗P�\����(�t���[����)</param>
				/// <param name="isSuicide">�Q�[���I�u�W�F�N�g�̍폜��ɂ��̃I�u�W�F�N�g�����E���邩?</param>
				CDeleteTimer(IGameObject* target, int timer, bool isSuicide = false) {
					Init(target, timer, isSuicide);
				}

				/// <summary>
				/// ������
				/// </summary>
				/// <param name="target">�폜����Q�[���I�u�W�F�N�g</param>
				/// <param name="timer">�폜�܂ł̗P�\����(�t���[����)</param>
				/// <param name="isSuicide">�Q�[���I�u�W�F�N�g�̍폜��ɂ��̃I�u�W�F�N�g�����E���邩?</param>
				void Init(IGameObject* target, int timer, bool isSuicide = false) {
					m_isSuicide = isSuicide;
					m_timer = timer;
					m_deleteTarget = target;
				}

				void PostUpdate()override {
					if (!m_deleteTarget) { return; }
					m_timer--;
					if (m_timer <= 0) {
						delete m_deleteTarget;
						if (m_isSuicide) { delete this; }
						return;
					}
				}

				/// <summary>
				/// �폜�܂ł̗P�\����(�t���[����)���擾
				/// </summary>
				int GetTimer()const { return m_timer; }

			private:
				bool m_isSuicide = false;
				int m_timer = -1;
				IGameObject* m_deleteTarget = nullptr;
			};

		}
	}
}