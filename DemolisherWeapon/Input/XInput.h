#pragma once

namespace DemolisherWeapon {

enum enXInputButton {
	enButtonUp,			//!<��B
	enButtonDown,		//!<���B
	enButtonLeft,		//!<���B
	enButtonRight,		//!<�E�B
	enButtonA,			//!<A�{�^���B
	enButtonB,			//!<B�{�^���B
	enButtonX,			//!<X�{�^���B
	enButtonY,			//!<Y�{�^���B
	enButtonBack,		//!<�o�b�N�{�^���B
	enButtonStart,		//!<�X�^�[�g�{�^���B
	enButtonRB1,		//!<RB1�{�^���B
	enButtonRT,			//!<RT �{�^���B
	enButtonRSB,		//!<RSB�{�^���B
	enButtonLB1,		//!<LB1�{�^���B
	enButtonLT,			//!<LT �{�^���B
	enButtonLSB,		//!<LSB�{�^���B
	enButtonNum,		//!<�{�^���̐��B
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
	void InLoopUpdate();
	
	//�{�^������
	bool GetButton(enXInputButton button) const;//���͏�Ԏ擾
	bool GetDown(enXInputButton button) const;//���̃t���[���ɉ����ꂽ�H
	bool GetUp(enXInputButton button) const;//���̃t���[���ɗ����ꂽ�H

	//�X�e�B�b�N���͒l�擾
	const CVector2& GetStick(enLR lr) const{
		if (!m_state.isConnect) { return CVector2::Zero(); }
		return m_state.m_stick[lr];
	}
	//�g���K�[���͒l�擾
	float GetTrigger(enLR lr) const{
		if (!m_state.isConnect) { return 0.0f; }
		return m_state.m_trigger[lr];
	}

	//�U��������
	void SetVibration(float L_vibration = 0.0f, float R_vibration = 0.0f) {
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed  = (WORD)(65535 * L_vibration);
		vib.wRightMotorSpeed = (WORD)(65535 * R_vibration);
		XInputSetState(m_padNum, &vib);
	};
	//�U��������(�������ԕt��)
	/*void SetVibrationWithTimeLimit(float L_vibration, float R_vibration, float timelimit) {
		SetVibration(L_vibration, R_vibration);
		m_vibrationTimelimit = timelimit;
	};*/

private:
	DWORD m_padNum;

	//�f�b�h�]�[���Y
	int m_LEFT_THUMB_DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 2;
	int m_RIGHT_THUMB_DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 2;// XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	int m_TRIGGER_THRESHOLD = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

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