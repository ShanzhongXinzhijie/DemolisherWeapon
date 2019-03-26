#pragma once
namespace DemolisherWeapon {
	namespace Physics2 {

		class MeshData;

		struct ReturnRayTest
		{
			bool isHit = false;
			const CVector3* vertexs[3];
			const CVector3* normal;
		};

		ReturnRayTest RayTest(const CVector3& start, const CVector3& end, const MeshData& mesh);

	}
}