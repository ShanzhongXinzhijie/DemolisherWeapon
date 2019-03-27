#pragma once
namespace DemolisherWeapon {
	namespace Physics2 {

		class MeshData;

		/// <summary>
		/// �����ƃ��b�V���̔���̖߂�l
		/// </summary>
		struct ReturnRayTest
		{
			bool isHit = false;
			CVector3 vertexs[3];
			CVector3 normal;
		};

		/// <summary>
		/// �����ƃ��b�V���̔���
		/// </summary>
		ReturnRayTest RayTest(const CVector3& start, const CVector3& end, const MeshData& mesh);

	}
}