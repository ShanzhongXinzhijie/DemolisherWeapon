#include "DWstdafx.h"
#include "FrustumCulling.h"

namespace DemolisherWeapon {

	bool FrustumCulling::AABBTest(GameObj::ICamera* camera, const CVector3& aabbMin, const CVector3& aabbMax) {

		//������J�����O
		//http://edom18.hateblo.jp/entry/2017/10/29/112908
		for (int i = 0; i < GameObj::ICamera::en6PlaneNum; i++) {
			const Plane& plane = camera->GetFrustumPlane(static_cast<GameObj::ICamera::Frustum6Plane>(i));

			//�@���ɑ΂��鐳�̒��_
			CVector3 vp;
			vp.x = plane.m_normal.x < 0.0f ? aabbMin.x : aabbMax.x;
			vp.y = plane.m_normal.y < 0.0f ? aabbMin.y : aabbMax.y;
			vp.z = plane.m_normal.z < 0.0f ? aabbMin.z : aabbMax.z;

			//���ʂ̕\���ɒ��_�����邩����
			float dp = plane.m_normal.Dot(vp - plane.m_position);
			if (dp < 0.0f){//�����ɂ���
				return false;//������ɓ����ĂȂ�
			}

			//float dn = planes[i].GetDistanceToPoint(vn);
			//if (dn < 0.0f){
			//	result = State.Intersect;//����
			//}
		}

		return true;//������ɓ����Ă���
	}

}