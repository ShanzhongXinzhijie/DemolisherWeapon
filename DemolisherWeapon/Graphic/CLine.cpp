#include "DWstdafx.h"
#include "CLine.h"

namespace DemolisherWeapon {

	void CLine::Draw(const CVector3& start, const CVector3& end, const CVector4& color) {
		GetGraphicsEngine().GetPrimitiveRender().AddLine(start, end, color, true, false);
	}

}