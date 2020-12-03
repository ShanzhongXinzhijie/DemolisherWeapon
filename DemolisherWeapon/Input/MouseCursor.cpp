#include "DWstdafx.h"
#include "MouseCursor.h"

namespace DemolisherWeapon {

CMouseCursor::CMouseCursor()
{
}


CMouseCursor::~CMouseCursor()
{
}


void CMouseCursor::Update() {
	//�N���C�A���g��̃J�[�\�����W���擾
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(GetEngine().GetWindowHandle(), &p);

	//�ۑ�
	CVector2 oldpos = m_mouseClientPos;
	m_mouseClientPos.x = (float)p.x;
	m_mouseClientPos.y = (float)p.y;
	m_mouseMove = m_mouseClientPos - oldpos;
	
	//0.0~1.0�ɕϊ���������
	m_mousePos.x = p.x / GetEngine().GetClientSize().x;
	m_mousePos.y = p.y / GetEngine().GetClientSize().y;

	//�J�[�\������ʒ����ɌŒ�
	if (m_lockCursor) {
		SetMouseCursor({ 0.5f,0.5f });

		//�J�[�\�����E�C���h�E����o�Ȃ��悤�ɐݒ�	
		POINT p = { 0,0 };
		ClientToScreen(GetEngine().GetWindowHandle(), &p);

		RECT rect = GetEngine().GetClientRECT();
		rect.left += p.x + 1;
		rect.right += p.x - 1;
		rect.top += p.y + 1;
		rect.bottom += p.y - 1;

		ClipCursor(&rect);
	}
}

//�}�E�X�J�[�\�����w��ʒu�ɃZ�b�g
void CMouseCursor::SetMouseCursor(const CVector2& vec) {
	//�X�N���[����̍��W�ɕϊ����ăZ�b�g
	POINT p = { (LONG)(GetEngine().GetClientSize().x*vec.x), (LONG)(GetEngine().GetClientSize().y*vec.y) };
	m_mouseClientPos.x = (float)p.x;
	m_mouseClientPos.y = (float)p.y;
	ClientToScreen(GetEngine().GetWindowHandle(), &p);
	SetCursorPos(p.x, p.y);

	m_mousePos = vec;
}

}