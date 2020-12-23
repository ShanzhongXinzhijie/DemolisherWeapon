#pragma once

namespace DemolisherWeapon {
	namespace GameObj {

		/// <summary>
		/// AABB‚ð•`‰æ‚·‚éƒNƒ‰ƒX
		/// </summary>
		class CAABBRender : public IGameObject {
		public:
			void Init(const CVector3& min, const CVector3& max, const CVector4& color) {
				m_min = min, m_max = max;
				m_color = color;
			}

		private:
			void PostLoopUpdate()override {
				DrawLine3D({ m_min.x,m_min.y,m_min.z }, { m_min.x,m_max.y,m_min.z }, m_color);
				DrawLine3D({ m_max.x,m_min.y,m_min.z }, { m_max.x,m_max.y,m_min.z }, m_color);
				DrawLine3D({ m_min.x,m_min.y,m_max.z }, { m_min.x,m_max.y,m_max.z }, m_color);
				DrawLine3D({ m_max.x,m_min.y,m_max.z }, { m_max.x,m_max.y,m_max.z }, m_color);

				DrawLine3D({ m_min.x,m_min.y,m_min.z }, { m_min.x,m_min.y,m_max.z }, m_color);
				DrawLine3D({ m_min.x,m_min.y,m_min.z }, { m_max.x,m_min.y,m_min.z }, m_color);
				DrawLine3D({ m_max.x,m_min.y,m_min.z }, { m_max.x,m_min.y,m_max.z }, m_color);
				DrawLine3D({ m_min.x,m_min.y,m_max.z }, { m_max.x,m_min.y,m_max.z }, m_color);

				DrawLine3D({ m_min.x,m_max.y,m_min.z }, { m_min.x,m_max.y,m_max.z }, m_color);
				DrawLine3D({ m_min.x,m_max.y,m_min.z }, { m_max.x,m_max.y,m_min.z }, m_color);
				DrawLine3D({ m_max.x,m_max.y,m_min.z }, { m_max.x,m_max.y,m_max.z }, m_color);
				DrawLine3D({ m_min.x,m_max.y,m_max.z }, { m_max.x,m_max.y,m_max.z }, m_color);
			}

		private:
			CVector3 m_min, m_max;
			CVector4 m_color = {1.0f,0.0f,0.0f,1.0f};
		};

	}
}