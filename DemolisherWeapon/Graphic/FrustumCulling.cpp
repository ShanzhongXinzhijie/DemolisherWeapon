#include "DWstdafx.h"
#include "FrustumCulling.h"

namespace DemolisherWeapon {

	bool FrustumCulling::AABBTest(GameObj::ICamera* camera, const CVector3& aabbMin, const CVector3& aabbMax) {
		//平行投影カメラの場合...

		//視錐台の各方向
		CVector3 vZ = GetMainCamera()->GetFront();
		CVector3 vX; vX.Cross(GetMainCamera()->GetUp(), vZ);
		CVector3 vY; vY.Cross(vZ, vX);

		//アスペクト比と視野角の取得
		float aspect = GetMainCamera()->GetAspect();
		float fov = GetMainCamera()->GetFOV();

		//近平面の高さと幅
		float nearPlaneHalfHeight = tanf(fov * 0.5f) * GetMainCamera()->GetNear();
		float nearPlaneHalfWidth = nearPlaneHalfHeight * aspect;

		//遠平面の高さと幅
		float farPlaneHalfHeight = tanf(fov * 0.5f) * GetMainCamera()->GetFar();
		float farPlaneHalfWidth = farPlaneHalfHeight * aspect;

		//近・遠平面の中心座標
		CVector3 nearPlaneCenter = GetMainCamera()->GetPos() + vZ * GetMainCamera()->GetNear();
		CVector3 farPlaneCenter = GetMainCamera()->GetPos() + vZ * GetMainCamera()->GetFar();

		//視錐台の各平面を求める
		//※法線は内向き
		CVector3 planes[6][2];//法線・平面上の一点
		CVector3 a, b, c;
		for (int i = 0; i < 6; i++) {
			if (i == 0) {//近平面
				a = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;//++
				b = nearPlaneCenter + vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;//+-
				c = nearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;//-+
			}
			if (i == 1) {//遠平面
				a = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;//-+
				b = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;//--
				c = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;//++
			}			
			if (i == 2) {//右
				a = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
				b = farPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
				c = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
			}
			if (i == 3) {//左
				a = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
				b = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
				c = nearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
			}			
			if (i == 4) {//下
				a = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
				b = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
				c = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
			}
			if (i == 5) {//上
				a = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
				b = nearPlaneCenter - vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;
				c = farPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
			}

			planes[i][0].Cross(b-a,c-a); planes[i][0] *= -1.0f; planes[i][0].Normalize();//法線
			planes[i][1] = a;//平面上の一点
		}
		//TODO ここの法線確認
		//AABB作成(モデル)
		//TODO カメラ側でやる?(毎回やらなくていい)

		//視錐台カリング
		//http://edom18.hateblo.jp/entry/2017/10/29/112908
		for (auto& plane : planes){

			//法線に対する正の頂点
			CVector3 vp;
			vp.x = plane[0].x < 0.0f ? aabbMin.x : aabbMax.x;
			vp.y = plane[0].y < 0.0f ? aabbMin.y : aabbMax.y;
			vp.z = plane[0].z < 0.0f ? aabbMin.z : aabbMax.z;

			//平面の表側に頂点があるか判定
			float dp = plane[0].Dot(vp - plane[1]);
			if (dp < 0.0f){//裏側にある
				return false;//視錐台に入ってない
			}

			//float dn = planes[i].GetDistanceToPoint(vn);
			//if (dn < 0.0f){
			//	result = State.Intersect;//交差
			//}
		}

		return true;//視錐台に入っている
	}

}