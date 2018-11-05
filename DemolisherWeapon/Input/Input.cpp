#include "DWstdafx.h"
#include "Input.h"

namespace DemolisherWeapon {
namespace GameObj {

float  DigitalInput::m_defaultDeadzone = 0.5f;

void DigitalInput::PreLoopUpdate() {
	switch (m_inputType)
	{
		bool input;

	case enMouseCursor:
		break;
	case enKeyState:
		input = GetKeyState().GetInput(m_keytype);
		break;
	case enMouseWheel:
		break;
	case enXInput:
		input = GetXInputManager().GetPad(0).GetButton(m_keytype);
		break;
	default:
		m_isPress = false, m_isTrigger = false;
		break;
	}
}

}
}