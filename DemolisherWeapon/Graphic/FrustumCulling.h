#pragma once

namespace DemolisherWeapon {

	class FrustumCulling
	{
	public:
		/// <summary>
		/// AABBに対して視錐台カリングテストを行う
		/// </summary>
		/// <param name="camera">カメラ</param>
		/// <param name="aabbMin">AABBの最小座標</param>
		/// <param name="aabbMax">AABBの最大座標</param>
		/// <returns>視錐台にAABBが入っているか？</returns>
		static bool AABBTest(GameObj::ICamera* camera, const CVector3& aabbMin, const CVector3& aabbMax);
	};

}
