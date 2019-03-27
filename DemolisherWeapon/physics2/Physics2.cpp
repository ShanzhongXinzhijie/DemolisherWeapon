#include "DWstdafx.h"
#include "Physics2.h"
#include "MeshData.h"

namespace DemolisherWeapon {
	namespace Physics2 {

		ReturnRayTest RayTest(const CVector3& start, const CVector3& end, const MeshData& mesh) {

			const MeshData::VertexBuffer& vertexBuffer = mesh.GetVertexBuffer();

			for (int i = 0; i < mesh.GetPolygonNum(); i++) {
			for (int i2 = 0; i2 < mesh.GetPolygonIndexs(i).size(); i2+=3) {
				//���_�Z�b�g
				CVector3 polygonVertex[3];
				polygonVertex[0] = vertexBuffer[mesh.GetPolygonIndexs(i).at(i2)];
				polygonVertex[1] = vertexBuffer[mesh.GetPolygonIndexs(i).at(i2+1)];
				polygonVertex[2] = vertexBuffer[mesh.GetPolygonIndexs(i).at(i2+2)];
				//�@�����߂�
				CVector3 polygonNormal;
				polygonNormal = polygonVertex[1] - polygonVertex[0];
				polygonNormal.Cross(polygonVertex[2] - polygonVertex[1]);
				polygonNormal.Normalize();

				//���������ʂ��ђʂ��Ă��邩�e�X�g
				CVector3 v1 = start - polygonVertex[0];
				CVector3 v2 = end - polygonVertex[0];
				if (v1.Dot(polygonNormal)*v2.Dot(polygonNormal) > FLT_EPSILON) {
					//�ђʂ��ĂȂ�
					//ReturnRayTest returnStruct;
					//returnStruct.isHit = false;
					//return returnStruct;
					continue;
				}

				//�ђʓ_�����߂�
				//����������߂�
				CVector3 vn = polygonNormal;// polygonVertex[0] + polygonNormal;
				float d1 = abs(vn.Dot(v1));// / vn.Length();
				float d2 = abs(vn.Dot(v2));// / vn.Length();
				float a = d1 / (d1 + d2);
				//�ђʓ_�����߂�
				CVector3 v3 = v1 * (1.0f - a) + v2 * a;
				CVector3 hitPos = polygonVertex[0] + v3;

				//�ђʓ_���|���S���̒��ɂ��邩����
				CVector3 c1 = polygonVertex[1] - polygonVertex[0]; c1.Cross(hitPos - polygonVertex[1]);
				CVector3 c2 = polygonVertex[2] - polygonVertex[1]; c2.Cross(hitPos - polygonVertex[2]);
				CVector3 c3 = polygonVertex[0] - polygonVertex[2]; c3.Cross(hitPos - polygonVertex[0]);

				//���ςŏ��������t���������ׂ�			
				if (c1.Dot(c2) > FLT_EPSILON && c1.Dot(c3) > FLT_EPSILON) {
					//�O�p�`�̓����ɓ_������
					ReturnRayTest returnStruct;
					returnStruct.isHit = true;
					returnStruct.normal = polygonNormal;
					returnStruct.vertexs[0] = polygonVertex[0];
					returnStruct.vertexs[1] = polygonVertex[1];
					returnStruct.vertexs[2] = polygonVertex[2];
					return returnStruct;
				}
			}
			}

			//������Ȃ�����
			ReturnRayTest returnStruct;
			returnStruct.isHit = false;
			return returnStruct;
		}

	}
}