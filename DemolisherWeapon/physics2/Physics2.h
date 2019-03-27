#pragma once
namespace DemolisherWeapon {
	namespace Physics2 {

		class MeshData;

		/// <summary>
		/// 線分とメッシュの判定の戻り値
		/// </summary>
		struct ReturnRayTest
		{
			bool isHit = false;
			CVector3 vertexs[3];
			CVector3 normal;
		};

		/// <summary>
		/// 線分とメッシュの判定
		/// </summary>
		ReturnRayTest RayTest(const CVector3& start, const CVector3& end, const MeshData& mesh);

	}
}