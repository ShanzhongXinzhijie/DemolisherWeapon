#pragma once

namespace DemolisherWeapon {

class MouseCursor
{
public:
	MouseCursor();
	~MouseCursor();

	void Update();
	//カーソル移動量のリセット
	void ResetMouseMove() {
		m_mouseMove = CVector2::Zero();
	}

	//マウスカーソルを画面中央に固定するか切り替え
	void SetLockMouseCursor(const bool b) { m_lockCursor = b; if (b == false) { ClipCursor(NULL); } }
	
	//マウスカーソルの表示状態を切り替え
	void SetShowMouseCursor(const bool b) {
		if (b) {
			//表示
			int cnt = ShowCursor(TRUE);
			while (cnt < 1) {
				cnt = ShowCursor(TRUE);
			}
		}
		else {
			//非表示
			int cnt = ShowCursor(FALSE);
			while (cnt > 0) {
				cnt = ShowCursor(FALSE);
			}
		}
	}

	//マウスカーソルを指定位置にセット
	void SetMouseCursor(const CVector2& vec);

	//カーソル位置(0.0~1.0)の取得
	CVector2 GetMouseCursorPos() { return m_mousePos; }
	//カーソル位置(クライアント座標)の取得
	CVector2 GetMouseCursorClientPos() { return m_mouseClientPos; }
	//カーソル移動量の取得
	CVector2 GetMouseMove() { return m_mouseMove; }

private:
	CVector2 m_mousePos, m_mouseClientPos;
	CVector2 m_mouseMove;
	bool m_lockCursor = false;
};

}