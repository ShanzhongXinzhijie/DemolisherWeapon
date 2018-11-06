#include "DWstdafx.h"
#include "Input.h"

namespace DemolisherWeapon {
namespace GameObj {

float  DigitalInput::m_defaultDeadzone = 0.5f;

void DigitalInput::PreLoopUpdate() {

	/*bool input = false;

	switch (m_inputType)
	{
	//case enMouseCursor:
	//	break;
	case enKeyState:
		input = GetKeyState().GetInput(m_keytype);
		break;
	//case enMouseWheel:
	//	break;
	case enXInput:
		switch (m_keytype) {
		case enButtonLT:
			if (GetXInputManager().GetPad(0).GetTrigger(L) >= m_deadzone) { input = true; }
			break;
		case enButtonRT:
			if (GetXInputManager().GetPad(padnum).GetTrigger(R) >= m_deadzone) { input = true; }
			break;
		default:
			for (const auto& vPadToXPad : vPadToXPadTable) {
				if (m_keytype == vPadToXPad.vButton) {
					input = GetXInputManager().GetPad(padnum).GetButton(vPadToXPad.xButton);
				}
			}
			break;
		}
		break;
	default:
		m_isPress = false;// , m_isTrigger = false;
		break;
	}

	m_isPress = input;*/
}

}
}