#pragma once

#include"CascadeShadowMap.h"

namespace DemolisherWeapon {

	//カスケードシャドウマップを使うためのクラス
	class CascadeShadowHandler
	{
	public:
		CascadeShadowHandler() = default;
		CascadeShadowHandler(int cascadeNum, const float splitCoefficient[], const CVector2 resolutions[]) { Init(cascadeNum, splitCoefficient, resolutions); }
		CascadeShadowHandler(int cascadeNum, const float splitCoefficient[]) { Init(cascadeNum, splitCoefficient); }
		CascadeShadowHandler(int cascadeNum) { Init(cascadeNum); }
		CascadeShadowHandler(int cascadeNum, const CVector3& dir) { Init(cascadeNum, dir); }

		~CascadeShadowHandler() { Release(); }

		//splitCoefficient ...cascadeNum+1個のfloat配列
		//resolutions ...cascadeNum個のCVector2配列
		void Init(int cascadeNum);
		void Init(int cascadeNum, const CVector3& dir) { Init(cascadeNum); SetDirection(dir); }
		void Init(int cascadeNum, const float splitCoefficient[]);
		void Init(int cascadeNum, const float splitCoefficient[], const CVector2 resolutions[]);
		
		void Release() {
			for (auto& sm : m_cShadowMaps) {
				sm->Release();
			}
			m_cShadowMaps.clear();
		}

		//ライトの方向を設定
		void SetDirection(const CVector3& direction) {
			for (auto& sm : m_cShadowMaps) {
				sm->SetLightDir(direction.GetNorm());
			}
		}

		void SetNear(float Near) {
			for (auto& sm : m_cShadowMaps) {
				sm->SetNear(Near);
			}
		}
		void SetFar(float Far) {
			for (auto& sm : m_cShadowMaps) {
				sm->SetFar(Far);
			}
		}

		//シャドウマップのしきい位置を変更
		void ChangeSplitCoefficients(const float splitCoefficients[]) {
			int i = 0;
			for (auto& sm : m_cShadowMaps) {
				sm->SetCascadeNear(splitCoefficients[i]);
				i++;
				sm->SetCascadeFar(splitCoefficients[i]);
			}
		}
		void SetSplitCoefficient(int index, float splitCoefficient) {
			int smindex = index / 2;
			if (index % 2 == 0) { 
				m_cShadowMaps[smindex]->SetCascadeNear(splitCoefficient);
				if (smindex - 1 >= 0) { m_cShadowMaps[smindex - 1]->SetCascadeFar(splitCoefficient); }
			}
			if (index % 2 != 0) {
				m_cShadowMaps[smindex]->SetCascadeFar(splitCoefficient);
				if (smindex + 1 < GetCascadeShadowMapNum()) { m_cShadowMaps[smindex + 1]->SetCascadeNear(splitCoefficient); }
			}
		}

		//シャドウマップの数を取得
		size_t GetCascadeShadowMapNum()const { return m_cShadowMaps.size(); }

		//シャドウマップを取得
		GameObj::CascadeShadowMap* GetCascadeShadowMap(int index) {
			return m_cShadowMaps[index].get();
		}

	private:
		std::vector<std::unique_ptr<GameObj::CascadeShadowMap>> m_cShadowMaps;
	};

}