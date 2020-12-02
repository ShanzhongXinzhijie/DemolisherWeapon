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
	//クライアント上のカーソル座標を取得
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(GetEngine().GetWindowHandle(), &p);

	//保存
	CVector2 oldpos = m_mouseClientPos;
	m_mouseClientPos.x = (float)p.x;
	m_mouseClientPos.y = (float)p.y;
	m_mouseMove = m_mouseClientPos - oldpos;
	
	//0.0~1.0に変換したもの
	m_mousePos.x = p.x / GetEngine().GetClientSize().x;
	m_mousePos.y = p.y / GetEngine().GetClientSize().y;

	//カーソルを画面中央に固定
	if (m_lockCursor) {
		SetMouseCursor({ 0.5f,0.5f });

		//カーソルをウインドウから出ないように設定	
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

//マウスカーソルを指定位置にセット
void CMouseCursor::SetMouseCursor(const CVector2& vec) {
	//スクリーン上の座標に変換してセット
	POINT p = { (LONG)(GetEngine().GetClientSize().x*vec.x), (LONG)(GetEngine().GetClientSize().y*vec.y) };
	m_mouseClientPos.x = (float)p.x;
	m_mouseClientPos.y = (float)p.y;
	ClientToScreen(GetEngine().GetWindowHandle(), &p);
	SetCursorPos(p.x, p.y);

	m_mousePos = vec;
}

}