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
		{ enButtonY			, XINPUT_GAMEPAD_Y },
		{ enButtonX			, XINPUT_GAMEPAD_X },
		{ enButtonSelect	, XINPUT_GAMEPAD_BACK },
		{ enButtonStart		, XINPUT_GAMEPAD_START },
		{ enButtonRB1		, XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{ enButtonRT		, 0 },
		{ enButtonRSB		, XINPUT_GAMEPAD_RIGHT_THUMB },
		{ enButtonLB1		, XINPUT_GAMEPAD_LEFT_SHOULDER },
		{ enButtonLT		, 0 },
		{ enButtonLSB		, XINPUT_GAMEPAD_LEFT_THUMB },
	};
}

bool XInputPad::GetButton(enXInputButton button) const {
	if (!m_state.isConnect) { return false; }//接続してない

	for (const auto& vPadToXPad : vPadToXPadTable) {
		if (button == vPadToXPad.vButton) {
			if (vPadToXPad.vButton == enButtonRT) {
				if (GetTrigger(R) > m_TRIGGER_THRESHOLD / 255.0f) {
					return true;
				}
				break;
			}
			if (vPadToXPad.vButton == enButtonLT) {
				if (GetTrigger(L) > m_TRIGGER_THRESHOLD / 255.0f) {
					return true;
				}
				break;
			}
			if ((m_state.state.Gamepad.wButtons & vPadToXPad.xButton ) != 0) {
				return true;
			}
			break;
		}
	}

	return false;
}

void XInputPad::Update() {
	DWORD dwResult;
	dwResult = XInputGetState(m_padNum, &m_state.state);

	if (dwResult == ERROR_SUCCESS)
	{
		m_state.isConnect = true;

		//デッドゾーンやんけ!
		if ((m_state.state.Gamepad.sThumbLX < m_LEFT_THUMB_DEADZONE &&
			m_state.state.Gamepad.sThumbLX > -m_LEFT_THUMB_DEADZONE) &&
			(m_state.state.Gamepad.sThumbLY < m_LEFT_THUMB_DEADZONE &&
				m_state.state.Gamepad.sThumbLY > -m_LEFT_THUMB_DEADZONE))
		{
			m_state.state.Gamepad.sThumbLX = 0;
			m_state.state.Gamepad.sThumbLY = 0;
		}
		if ((m_state.state.Gamepad.sThumbRX < m_RIGHT_THUMB_DEADZONE &&
			m_state.state.Gamepad.sThumbRX > -m_RIGHT_THUMB_DEADZONE) &&
			(m_state.state.Gamepad.sThumbRY < m_RIGHT_THUMB_DEADZONE &&
				m_state.state.Gamepad.sThumbRY > -m_RIGHT_THUMB_DEADZONE))
		{
			m_state.state.Gamepad.sThumbRX = 0;
			m_state.state.Gamepad.sThumbRY = 0;
		}

		m_state.m_stick[L].x = m_state.state.Gamepad.sThumbLX / 32768.0f;
		m_state.m_stick[L].y = m_state.state.Gamepad.sThumbLY / 32768.0f;
		m_state.m_stick[R].x = m_state.state.Gamepad.sThumbRX / 32768.0f;
		m_state.m_stick[R].y = m_state.state.Gamepad.sThumbRY / 32768.0f;		

		m_state.m_trigger[L] = m_state.state.Gamepad.bLeftTrigger  / 255.0f;
		m_state.m_trigger[R] = m_state.state.Gamepad.bRightTrigger / 255.0f;		
	}
	else
	{
		m_state.isConnect = false;
		m_vibrationTimelimit = 0.0f;
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