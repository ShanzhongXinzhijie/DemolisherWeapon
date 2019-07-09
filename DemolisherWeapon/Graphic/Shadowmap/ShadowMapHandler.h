#pragma once

namespace DemolisherWeapon {

	class ShadowMapHandler
	{
	public:
		ShadowMapHandler() = default;
		~ShadowMapHandler();

		void ShadowMapUpdate();

		//初期化
		//UINT width, UINT height ...シャドウマップの解像度
		void Init(UINT width, UINT height, const CVector3& direction, const CVector3& target = CVector3::Zero(), const CVector3& area = { 100.0f, 100.0f, 100.0f }, float Near = 0.0f, const CVector3& dir_y = CVector3::Zero());
		void Release();

		//シャドウマップの範囲の中心位置を設定
		void SetTarget(const CVector3& target) {
			m_target = target;
			ShadowMapUpdate();
		}
		//ライトの方向を設定
		void SetDirection(const CVector3& direction) {
			m_direction = direction; m_direction.Normalize();
			ShadowMapUpdate();
		}
		void SetNear(float Near) {
			m_near = Near;
			ShadowMapUpdate();
		}
		//シャドウマップの範囲を設定(Yが上方向、Zがライトの方向)
		void SetArea(const CVector3& area) {
			m_area = area;
			ShadowMapUpdate();
		}
		//上方向を設定
		void SetY(const CVector3& dir_y) {
			m_dirY = dir_y;	m_dirY.Normalize();
			ShadowMapUpdate();
		}

		//深度バイアスを設定
		void SetDepthBias(float bias) {
			m_shadowMap->SetDepthBias(bias);
		}

		//更新間隔を設定
		void SetInterval(int interval) { m_shadowMap->SetRenderInterval(interval); }

		//更新するか設定
		void SetIsUpdate(bool isUpdate) { m_shadowMap->SetIsUpdate(isUpdate); }

	private:
		CShadowMap* m_shadowMap = nullptr;
		CVector3 m_direction, m_target, m_area, m_dirY;
		float m_near = 0.0f;
	};

}