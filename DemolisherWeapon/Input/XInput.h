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
	enButtonBack,		//!<バックボタン。
	enButtonStart,		//!<スタートボタン。
	enButtonRB1,		//!<RB1ボタン。
	enButtonRT,			//!<RT ボタン。
	enButtonRSB,		//!<RSBボタン。
	enButtonLB1,		//!<LB1ボタン。
	enButtonLT,			//!<LT ボタン。
	enButtonLSB,		//!<LSBボタン。

	enButtonLSUp,		//!<Lスティック上。
	enButtonLSDown,		//!<Lスティック下。
	enButtonLSLeft,		//!<Lスティック左。
	enButtonLSRight,	//!<Lスティック右。

	enButtonRSUp,		//!<Rスティック上。
	enButtonRSDown,		//!<Rスティック下。
	enButtonRSLeft,		//!<Rスティック左。
	enButtonRSRight,	//!<Rスティック右。

	enButtonNum,		//!<ボタンの数。
};
constexpr inline auto enButtonRB = enButtonRB1;
constexpr inline auto enButtonLB = enButtonLB1;

//左右
enum enLR {
	L, R, enLRNUM,
};
static constexpr enLR LR[] = { L,R };

//スティック入力の最大値
static constexpr float STICK_INPUT_MAX = 32768.0f;
//トリガー入力の最大値
static constexpr float TRRIGER_INPUT_MAX = 255.0f;

class XInputPad
{
public:	
	void Init(DWORD padnum) {
		m_padNum = padnum;
	}

	void Update();
	void InLoopUpdate();
	
	//ボタン入力
	bool GetButton(enXInputButton button) const;//入力状態取得
	bool GetDown(enXInputButton button) const;//このフレームに押された？
	bool GetUp(enXInputButton button) const;//このフレームに離された？

	//何かしらのボタンが押された?
	bool GetAnyDown()const {
		for (int i = 0; i < enButtonNum; i++) {
			if (GetDown((enXInputButton)i)) {
				return true;
			}
		}
		return false;
	}

	//スティック入力値取得
	const CVector2& GetStick(enLR lr) const{
		if (!m_state.isConnect) { return CVector2::Zero(); }
		return m_state.m_stick[lr];
	}
	//トリガー入力値取得
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
	int m_LEFT_THUMB_DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 2;
	int m_RIGHT_THUMB_DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 2;// XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	int m_TRIGGER_THRESHOLD = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	//アナログ入力をボタン入力として扱うしきい値
	float m_LSButtonThreshold = 0.5f;
	float m_RSButtonThreshold = 0.5f;
	float m_TriggerButtonThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD / TRRIGER_INPUT_MAX;

	struct PadState{
		bool isConnect = false;
		XINPUT_STATE state;
		CVector2 m_stick[enLRNUM];
		float m_trigger[enLRNUM] = { 0 };
	};

	PadState m_state;
	bool m_buttonInputOld[enButtonNum] = { 0 };
	//float m_vibrationTimelimit = 0.0f;
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
	void InLoopUpdate() {
		for (int i = 0; i < MAX_CONTROLLERS; i++) {
			m_pad[i].InLoopUpdate();
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