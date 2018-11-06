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
	m_mousePos.x = p.x / GetEngine().GetWindowSize().x;
	m_mousePos.y = p.y / GetEngine().GetWindowSize().y;

	//カーソルを画面中央に固定
	if (m_lockCursor) {
		SetMouseCursor({ 0.5f,0.5f });
		//カーソルをウインドウから出ないように設定		
		ClipCursor(&GetEngine().GetWindowRECT());
	}
}

//マウスカーソルを指定位置にセット
void CMouseCursor::SetMouseCursor(const CVector2& vec) {
	//スクリーン上の座標に変換してセット
	POINT p = { (LONG)(GetEngine().GetWindowSize().x*vec.x), (LONG)(GetEngine().GetWindowSize().y*vec.y) };
	m_mouseClientPos.x = (float)p.x;
	m_mouseClientPos.y = (float)p.y;
	ClientToScreen(GetEngine().GetWindowHandle(), &p);
	SetCursorPos(p.x, p.y);

	m_mousePos = vec;
}

}