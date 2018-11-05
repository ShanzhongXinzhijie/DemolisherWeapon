#include "DWstdafx.h"
#include "KeyState.h"


KeyState::KeyState()
{
}


KeyState::~KeyState()
{
}

void KeyState::Update() {
	GetKeyboardState(m_key);
}

bool KeyState::GetInput(const int n) {
	if (m_key[n] & 0x80) {
		return true;
	}
	return false;
}
