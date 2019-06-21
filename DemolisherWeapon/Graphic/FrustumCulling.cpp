#include "DWstdafx.h"
#include "FrustumCulling.h"

namespace DemolisherWeapon {

	bool FrustumCulling::AABBTest(GameObj::ICamera* camera, const CVector3& aabbMin, const CVector3& aabbMax) {
		//���s���e�J�����̏ꍇ...

		//������̊e����
		CVector3 vZ = GetMainCamera()->GetFront();
		CVector3 vX; vX.Cross(GetMainCamera()->GetUp(), vZ);
		CVector3 vY; vY.Cross(vZ, vX);

		//�A�X�y�N�g��Ǝ���p�̎擾
		float aspect = GetMainCamera()->GetAspect();
		float fov = GetMainCamera()->GetFOV();

		//�ߕ��ʂ̍����ƕ�
		float nearPlaneHalfHeight = tanf(fov * 0.5f) * GetMainCamera()->GetNear();
		float nearPlaneHalfWidth = nearPlaneHalfHeight * aspect;

		//�����ʂ̍����ƕ�
		float farPlaneHalfHeight = tanf(fov * 0.5f) * GetMainCamera()->GetFar();
		float farPlaneHalfWidth = farPlaneHalfHeight * aspect;

		//�߁E�����ʂ̒��S���W
		CVector3 nearPlaneCenter = GetMainCamera()->GetPos() + vZ * GetMainCamera()->GetNear();
		CVector3 farPlaneCenter = GetMainCamera()->GetPos() + vZ * GetMainCamera()->GetFar();

		//������̊e���ʂ����߂�
		//���@���͓�����
		CVector3 planes[6][2];//�@���E���ʏ�̈�_
		CVector3 a, b, c;
		for (int i = 0; i < 6; i++) {
			if (i == 0) {//�ߕ���
				a = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;//++
				b = nearPlaneCenter + vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;//+-
				c = nearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;//-+
			}
			if (i == 1) {//������
				a = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;//-+
				b = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;//--
				c = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;//++
			}			
			if (i == 2) {//�E
				a = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
				b = farPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
				c = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
			}
			if (i == 3) {//��
				a = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
				b = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
				c = nearPlaneCenter - vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
			}			
			if (i == 4) {//��
				a = farPlaneCenter + vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
				b = nearPlaneCenter + vX * nearPlaneHalfWidth + vY * nearPlaneHalfHeight;
				c = farPlaneCenter - vX * farPlaneHalfWidth + vY * farPlaneHalfHeight;
			}
			if (i == 5) {//��
				a = farPlaneCenter - vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
				b = nearPlaneCenter - vX * nearPlaneHalfWidth - vY * nearPlaneHalfHeight;
				c = farPlaneCenter + vX * farPlaneHalfWidth - vY * farPlaneHalfHeight;
			}

			planes[i][0].Cross(b-a,c-a); planes[i][0] *= -1.0f; planes[i][0].Normalize();//�@��
			planes[i][1] = a;//���ʏ�̈�_
		}
		//TODO �����̖@���m�F
		//AABB�쐬(���f��)
		//TODO �J�������ł��?(������Ȃ��Ă���)

		//������J�����O
		//http://edom18.hateblo.jp/entry/2017/10/29/112908
		for (auto& plane : planes){

			//�@���ɑ΂��鐳�̒��_
			CVector3 vp;
			vp.x = plane[0].x < 0.0f ? aabbMin.x : aabbMax.x;
			vp.y = plane[0].y < 0.0f ? aabbMin.y : aabbMax.y;
			vp.z = plane[0].z < 0.0f ? aabbMin.z : aabbMax.z;

			//���ʂ̕\���ɒ��_�����邩����
			float dp = plane[0].Dot(vp - plane[1]);
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