#pragma once

namespace DemolisherWeapon {

enum enXInputButton {
	enButtonUp,			//!<上。
	enButtonDown,		//!<下。
	enButtonLeft,		//!<左。
	enButtonRight,		//!<右。
	enButtonA,			//!<Aボタン。
	enButtonB,			//!<Bボタン。
	enButtonX,			//!<Xボタン。
	enButtonY,			//!<Yボタン。
	enButtonSelect,		//!<セレクトボタン。
	enButtonStart,		//!<スタートボタン。
	enButtonRB1,		//!<RB1ボタン。
	enButtonRT,			//!<RT ボタン。
	enButtonRSB,		//!<RSBボタン。
	enButtonLB1,		//!<LB1ボタン。
	enButtonLT,			//!<LT ボタン。
	enButtonLSB,		//!<LSBボタン。
	enButtonNum,		//!<ボタンの数。
};

enum enLR {
	L, R, enLRNUM,
};

class XInputPad
{
public:	

	void Init(DWORD padnum) {
		m_padNum = padnum;
	}

	void Update();

	//入力を取得
	bool GetButton(enXInputButton button) const;
	//bool IsTrigger(enXInputButton button) const;

	CVector2 GetStick(enLR lr) const{
		if (!m_state.isConnect) { return CVector2::Zero(); }
		return m_state.m_stick[lr];
	}
	float GetTrigger(enLR lr) const{
		if (!m_state.isConnect) { return 0.0f; }
		return m_state.m_trigger[lr];
	}

	//振動させる
	void SetVibration(float L_vibration = 0.0f, float R_vibration = 0.0f) {
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed  = (WORD)(65535 * L_vibration);
		vib.wRightMotorSpeed = (WORD)(65535 * R_vibration);
		XInputSetState(m_padNum, &vib);
	};
	//振動させる(制限時間付き)
	/*void SetVibrationWithTimeLimit(float L_vibration, float R_vibration, float timelimit) {
		SetVibration(L_vibration, R_vibration);
		m_vibrationTimelimit = timelimit;
	};*/

private:
	DWORD m_padNum;

	//デッドゾーンズ
	int m_LEFT_THUMB_DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	int m_RIGHT_THUMB_DEADZONE = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	int m_TRIGGER_THRESHOLD = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

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

}