#pragma once

namespace DemolisherWeapon {

class MouseCursor
{
public:
	MouseCursor();
	~MouseCursor();

	void Update();
	//�J�[�\���ړ��ʂ̃��Z�b�g
	void ResetMouseMove() {
		m_mouseMove = CVector2::Zero();
	}

	//�}�E�X�J�[�\������ʒ����ɌŒ肷�邩�؂�ւ�
	void SetLockMouseCursor(const bool b) { m_lockCursor = b; if (b == false) { ClipCursor(NULL); } }
	
	//�}�E�X�J�[�\���̕\����Ԃ�؂�ւ�
	void SetShowMouseCursor(const bool b) {
		if (b) {
			//�\��
			int cnt = ShowCursor(TRUE);
			while (cnt < 1) {
				cnt = ShowCursor(TRUE);
			}
		}
		else {
			//��\��
			int cnt = ShowCursor(FALSE);
			while (cnt > 0) {
				cnt = ShowCursor(FALSE);
			}
		}
	}

	//�}�E�X�J�[�\�����w��ʒu�ɃZ�b�g
	void SetMouseCursor(const CVector2& vec);

	//�J�[�\���ʒu(0.0~1.0)�̎擾
	CVector2 GetMouseCursorPos() { return m_mousePos; }
	//�J�[�\���ʒu(�N���C�A���g���W)�̎擾
	CVector2 GetMouseCursorClientPos() { return m_mouseClientPos; }
	//�J�[�\���ړ��ʂ̎擾
	CVector2 GetMouseMove() { return m_mouseMove; }

private:
	CVector2 m_mousePos, m_mouseClientPos;
	CVector2 m_mouseMove;
	bool m_lockCursor = false;
};

}