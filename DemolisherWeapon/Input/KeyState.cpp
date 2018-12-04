#include "DWstdafx.h"
#include "KeyState.h"

namespace DemolisherWeapon {

	KeyState::KeyState()
	{
	}


	KeyState::~KeyState()
	{
	}

	void KeyState::Update() {
		GetKeyboardState(m_key);
	}
	void KeyState::InLoopUpdate() {
		for (int i = 0; i < 256; i++) {
			m_keyOld[i] = m_key[i];
		}
	}

	bool KeyState::GetInput(const int n) const{
		if (m_key[n] & 0x80) {
			return true;
		}
		return false;
	}

	bool KeyState::GetDown(const int n) const{
		if (!(m_keyOld[n] & 0x80) && m_key[n] & 0x80) {
			return true;
		}
		return false;
	}

	bool KeyState::GetUp(const int n) const{
		if (m_keyOld[n] & 0x80 && !(m_key[n] & 0x80)) {
			return true;
		}
		return false;
	}

}