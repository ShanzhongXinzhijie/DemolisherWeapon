#pragma once

class XInputPad
{
public:
	enum enLR {
		L, R, enLRNUM,
	};

	void Init(DWORD padnum) {
		m_padNum = padnum;
	}

	void Update();

	bool GetButton(DWORD button) const{
		if (!m_state.isConnect) { return false; }
		if ((m_state.state.Gamepad.wButtons & button) != 0) {
			return true;
		}
		return false;
	}
	CVector2 GetStick(enLR lr) const{
		if (!m_state.isConnect) { return CVector2::Zero(); }
		return m_state.m_stick[lr];
	}
	float GetTrigger(enLR lr) const{
		if (!m_state.isConnect) { return 0.0f; }
		return m_state.m_trigger[lr];
	}

	//êUìÆÇ≥ÇπÇÈ
	void SetVibration(float L_vibration = 0.0f, float R_vibration = 0.0f) {
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed  = (WORD)(65535 * L_vibration);
		vib.wRightMotorSpeed = (WORD)(65535 * R_vibration);
		XInputSetState(m_padNum, &vib);
	};
	//êUìÆÇ≥ÇπÇÈ(êßå¿éûä‘ïtÇ´)
	/*void SetVibrationWithTimeLimit(float L_vibration, float R_vibration, float timelimit) {
		SetVibration(L_vibration, R_vibration);
		m_vibrationTimelimit = timelimit;
	};*/

private:
	DWORD m_padNum;

	struct PadState{
		bool isConnect = false;
		XINPUT_STATE state;
		CVector2 m_stick[enLRNUM];
		float m_trigger[enLRNUM] = { 0 };
	};

	PadState m_state;
	float m_vibrationTimelimit = 0.0f;
};

class XInputManager {
public:
	XInputManager(){
		for (int i = 0; i < MAX_CONTROLLERS; i++){
			m_pad[i].Init(i);
		}
	}

	void Update() {
		for (int i = 0; i < MAX_CONTROLLERS; i++) {
			m_pad[i].Update();
		}
	}
	XInputPad& GetPad(int n) {
		return m_pad[n];
	}
private:
	static const int MAX_CONTROLLERS = 4;

	XInputPad m_pad[MAX_CONTROLLERS];
};
