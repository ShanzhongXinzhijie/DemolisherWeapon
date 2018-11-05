#pragma once

namespace DemolisherWeapon {
namespace GameObj {

class DigitalInput : public IGameObject
{
private:

	enum enInputType {
		enNoSet,
		enMouseCursor,
		enMouseWheel,
		enKeyState,
		enXInput,
	};

public:

	void PreLoopUpdate()override;

	//�g�p������͕��@��ݒ�
	void SetUseInput(enInputType inputtype, int keytype) {
		m_inputType = inputtype;
		m_keytype = keytype;
	}

	//�L�[�R���t�B�O
	//void StartKeyConfig();
	//bool IsRunKeyConfig();

	void SetDeadZone(float deadzone);
	static void SetDefaultDeadZone(float deadzone);
	//�}�E�X�J�[�\���E�z�C�[��

	//���͏�Ԃ��擾
	bool GetPress()const { return m_isPress; }
	bool GetTrigeer()const { return m_isTrigger; }

private:

	enInputType m_inputType = enNoSet;
	int m_keytype = 0;

	bool m_isPress = false, m_isTrigger = false;

	static float m_defaultDeadzone;
	float m_deadzone = m_defaultDeadzone;
};

class AnalogInput : public IGameObject
{
public:
	void SetUseInput();
	
	float Get();
	float GetDiff();
};

}
}