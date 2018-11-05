#include "DWstdafx.h"
#include "GameObject.h"

namespace DemolisherWeapon {

	IGameObject::IGameObject(bool isRegister) {
		if (isRegister) { AddGO(this); }
	}

	void GOStatusCaster::Cast(const GOStatus& status) {
		m_receiver->SetStatus(status);
	}

}