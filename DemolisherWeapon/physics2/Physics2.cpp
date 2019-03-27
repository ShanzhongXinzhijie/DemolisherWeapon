#include "DWstdafx.h"
#include "Physics2.h"
#include "MeshData.h"

namespace DemolisherWeapon {
	namespace Physics2 {

		ReturnRayTest RayTest(const CVector3& start, const CVector3& end, const MeshData& mesh) {

			const MeshData::VertexBuffer& vertexBuffer = mesh.GetVertexBuffer();

			for (int i = 0; i < mesh.GetPolygonNum(); i++) {
			for (int i2 = 0; i2 < mesh.GetPolygonIndexs(i).size(); i2+=3) {
				//頂点セット
				CVector3 polygonVertex[3];
				polygonVertex[0] = vertexBuffer[mesh.GetPolygonIndexs(i).at(i2)];
				polygonVertex[1] = vertexBuffer[mesh.GetPolygonIndexs(i).at(i2+1)];
				polygonVertex[2] = vertexBuffer[mesh.GetPolygonIndexs(i).at(i2+2)];
				//法線求める
				CVector3 polygonNormal;
				polygonNormal = polygonVertex[1] - polygonVertex[0];
				polygonNormal.Cross(polygonVertex[2] - polygonVertex[1]);
				polygonNormal.Normalize();

				//線分が平面を貫通しているかテスト
				CVector3 v1 = start - polygonVertex[0];
				CVector3 v2 = end - polygonVertex[0];
				if (v1.Dot(polygonNormal)*v2.Dot(polygonNormal) > FLT_EPSILON) {
					//貫通してない
					//ReturnRayTest returnStruct;
					//returnStruct.isHit = false;
					//return returnStruct;
					continue;
				}

				//貫通点を求める
				//内分比を求める
				CVector3 vn = polygonNormal;// polygonVertex[0] + polygonNormal;
				float d1 = abs(vn.Dot(v1));// / vn.Length();
				float d2 = abs(vn.Dot(v2));// / vn.Length();
				float a = d1 / (d1 + d2);
				//貫通点を求める
				CVector3 v3 = v1 * (1.0f - a) + v2 * a;
				CVector3 hitPos = polygonVertex[0] + v3;

				//貫通点がポリゴンの中にあるか判定
				CVector3 c1 = polygonVertex[1] - polygonVertex[0]; c1.Cross(hitPos - polygonVertex[1]);
				CVector3 c2 = polygonVertex[2] - polygonVertex[1]; c2.Cross(hitPos - polygonVertex[2]);
				CVector3 c3 = polygonVertex[0] - polygonVertex[2]; c3.Cross(hitPos - polygonVertex[0]);

				//内積で順方向か逆方向か調べる			
				if (c1.Dot(c2) > FLT_EPSILON && c1.Dot(c3) > FLT_EPSILON) {
					//三角形の内側に点がある
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

			//当たらなかった
			ReturnRayTest returnStruct;
			returnStruct.isHit = false;
			return returnStruct;
		}

	}
}