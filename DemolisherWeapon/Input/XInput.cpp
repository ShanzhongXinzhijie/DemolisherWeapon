#include "DWstdafx.h"
#include "XInput.h"


void XInputPad::Update() {
	DWORD dwResult;
	dwResult = XInputGetState(m_padNum, &m_state.state);

	if (dwResult == ERROR_SUCCESS)
	{
		m_state.isConnect = true;

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
	//ƒ^ƒCƒ€ƒŠƒ~ƒbƒg‚ðŒ}‚¦‚½‚çU“®Ž~‚ß‚é
	if (m_vibrationTimelimit > 0.0f) {
		m_vibrationTimelimit -= sec;
		if (m_vibrationTimelimit < FLT_EPSILON) {
			SetVibration(0.0f, 0.0f);
			m_vibrationTimelimit = 0.0f;
		}
	}
}*/