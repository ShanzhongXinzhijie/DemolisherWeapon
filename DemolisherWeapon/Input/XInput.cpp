#include "DWstdafx.h"
#include "XInput.h"

namespace DemolisherWeapon {

namespace {
	struct VirtualPadToXPad {
		enXInputButton vButton;		//!<仮想ボタン。
		DWORD		   xButton;		//!<XBoxコントローラのボタン。
	};
	const VirtualPadToXPad vPadToXPadTable[enButtonNum] = {
		{ enButtonUp		, XINPUT_GAMEPAD_DPAD_UP },
		{ enButtonDown		, XINPUT_GAMEPAD_DPAD_DOWN },
		{ enButtonLeft		, XINPUT_GAMEPAD_DPAD_LEFT },
		{ enButtonRight		, XINPUT_GAMEPAD_DPAD_RIGHT },
		{ enButtonA			, XINPUT_GAMEPAD_A },
		{ enButtonB			, XINPUT_GAMEPAD_B },
		{ enButtonX			, XINPUT_GAMEPAD_X },
		{ enButtonY			, XINPUT_GAMEPAD_Y },
		{ enButtonBack		, XINPUT_GAMEPAD_BACK },
		{ enButtonStart		, XINPUT_GAMEPAD_START },
		{ enButtonRB1		, XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{ enButtonRT		, 0 },
		{ enButtonRSB		, XINPUT_GAMEPAD_RIGHT_THUMB },
		{ enButtonLB1		, XINPUT_GAMEPAD_LEFT_SHOULDER },
		{ enButtonLT		, 0 },
		{ enButtonLSB		, XINPUT_GAMEPAD_LEFT_THUMB },

		{ enButtonLSUp		, 0 },
		{ enButtonLSDown	, 0 },
		{ enButtonLSLeft	, 0 },
		{ enButtonLSRight	, 0 },

		{ enButtonRSUp		, 0 },
		{ enButtonRSDown	, 0 },
		{ enButtonRSLeft	, 0 },
		{ enButtonRSRight	, 0 },
	};
}

bool XInputPad::GetButton(enXInputButton button) const {
	if (!m_state.isConnect) { return false; }//接続してない

	//テーブルから指定のボタンを取得
	const auto& vPadToXPad = vPadToXPadTable[button];
	
	//トリガーの場合
	if (vPadToXPad.vButton == enButtonRT) {
		if (GetTrigger(R) > m_TriggerButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonLT) {
		if (GetTrigger(L) > m_TriggerButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}

	//スティックの場合

	//L
	if (vPadToXPad.vButton == enButtonLSUp) {
		if (GetStick(L).y > m_LSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonLSDown) {
		if (GetStick(L).y < -m_LSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonLSLeft) {
		if (GetStick(L).x < -m_LSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonLSRight) {
		if (GetStick(L).x > m_LSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}

	//R
	if (vPadToXPad.vButton == enButtonRSUp) {
		if (GetStick(R).y > m_RSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonRSDown) {
		if (GetStick(R).y < -m_RSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonRSLeft) {
		if (GetStick(R).x < -m_RSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}
	if (vPadToXPad.vButton == enButtonRSRight) {
		if (GetStick(R).x > m_RSButtonThreshold) {
			return true;//押されている
		}
		return false;//押されていない
	}

	//ボタンの場合
	if ((m_state.state.Gamepad.wButtons & vPadToXPad.xButton ) != 0) {
		return true;//押されている
	}	
	return false;//押されていない
}
bool XInputPad::GetDown(enXInputButton button) const {
	if (!m_buttonInputOld[button] && GetButton(button)) {
		return true;
	}
	return false;
}
bool XInputPad::GetUp(enXInputButton button) const {
	if (m_buttonInputOld[button] && !GetButton(button)) {
		return true;
	}
	return false;
}

void XInputPad::InLoopUpdate() {
	for (int i = 0; i < enButtonNum; i++) {
		m_buttonInputOld[i] = GetButton(vPadToXPadTable[i].vButton);
	}
}

void XInputPad::Update() {
	DWORD dwResult;
	dwResult = XInputGetState(m_padNum, &m_state.state);

	HWND hWnd = GetActiveWindow();

	if (dwResult == ERROR_SUCCESS && hWnd == GetEngine().GetWindowHandle())
	{
		m_state.isConnect = true;

		//デッドゾーンやんけ!
		CVector2 v_stick;
		/*if ((m_state.state.Gamepad.sThumbLX < m_LEFT_THUMB_DEADZONE &&
			m_state.state.Gamepad.sThumbLX > -m_LEFT_THUMB_DEADZONE) &&
			(m_state.state.Gamepad.sThumbLY < m_LEFT_THUMB_DEADZONE &&
				m_state.state.Gamepad.sThumbLY > -m_LEFT_THUMB_DEADZONE))*/
		v_stick = { (float)m_state.state.Gamepad.sThumbLX, (float)m_state.state.Gamepad.sThumbLY };
		if(v_stick.LengthSq() < m_LEFT_THUMB_DEADZONE*m_LEFT_THUMB_DEADZONE)
		{
			m_state.state.Gamepad.sThumbLX = 0;
			m_state.state.Gamepad.sThumbLY = 0;
		}
		/*if ((m_state.state.Gamepad.sThumbRX < m_RIGHT_THUMB_DEADZONE &&
			m_state.state.Gamepad.sThumbRX > -m_RIGHT_THUMB_DEADZONE) &&
			(m_state.state.Gamepad.sThumbRY < m_RIGHT_THUMB_DEADZONE &&
				m_state.state.Gamepad.sThumbRY > -m_RIGHT_THUMB_DEADZONE))*/
		v_stick = { (float)m_state.state.Gamepad.sThumbRX, (float)m_state.state.Gamepad.sThumbRY };
		if (v_stick.LengthSq() < m_RIGHT_THUMB_DEADZONE*m_RIGHT_THUMB_DEADZONE)
		{
			m_state.state.Gamepad.sThumbRX = 0;
			m_state.state.Gamepad.sThumbRY = 0;
		}

		m_state.m_stick[L].x = m_state.state.Gamepad.sThumbLX / STICK_INPUT_MAX;
		m_state.m_stick[L].y = m_state.state.Gamepad.sThumbLY / STICK_INPUT_MAX;
		m_state.m_stick[R].x = m_state.state.Gamepad.sThumbRX / STICK_INPUT_MAX;
		m_state.m_stick[R].y = m_state.state.Gamepad.sThumbRY / STICK_INPUT_MAX;

		m_state.m_trigger[L] = m_state.state.Gamepad.bLeftTrigger  / TRRIGER_INPUT_MAX;
		m_state.m_trigger[R] = m_state.state.Gamepad.bRightTrigger / TRRIGER_INPUT_MAX;
	}
	else
	{
		m_state.isConnect = false;
		//m_vibrationTimelimit = 0.0f;
	}	
}

/*void XInputPad::UpdateVibration(float sec) {
	//タイムリミットを迎えたら振動止める
	if (m_vibrationTimelimit > 0.0f) {
		m_vibrationTimelimit -= sec;
		if (m_vibrationTimelimit < FLT_EPSILON) {
			SetVibration(0.0f, 0.0f);
			m_vibrationTimelimit = 0.0f;
		}
	}
}*/

}