#pragma once

#include "CEnemy.h"

class CPlayer : public IGameObject {
public:
	bool Start()override {
		m_animationClips[0].Load(L"Resource/animData/run.tka", true, enYtoZ);
		m_animationClips[1].Load(L"Resource/animData/violence.tka",false);

		m_model.Init(L"Resource/modelData/unityChan.cmo", m_animationClips, 2);		
		m_rot.SetRotationDeg(CVector3::AxisY(), 90.0f);

		return true;
	};

	void Update()override {

		//左スティックで移動
		m_pos.x += Pad(0).GetStick(L).x*40.0f;
		m_pos.z += Pad(0).GetStick(L).y*40.0f;

		//Aボタンで攻撃
		if (Pad(0).GetButton(enButtonA)) {		
			if (!m_trigger) {
				//攻撃判定の発生
				SuicideObj::CCollisionObj* attackCol = NewGO<SuicideObj::CCollisionObj>();
				//形状の作成
				attackCol->CreateSphere(m_pos + CVector3::AxisX()*50.0f, CQuaternion::Identity(), 100.0f);
				//寿命を設定
				attackCol->SetTimer(10);//10フレーム後削除される
				//衝突時に実行する処理を設定
				attackCol->SetCallback([&](SuicideObj::CCollisionObj::SCallbackParam& param) {
					//衝突した判定の名前が"CEnemy"なら5ダメージ与える
					if (param.EqualName(L"CEnemy")) {
						CEnemy* enemy = param.GetClass<CEnemy>();//相手の判定に設定されているCEnemyのポインタを取得
						enemy->Damege(5);
					}
				}
				);
			}
			m_trigger = true;
		}
		else {
			m_trigger = false;
		}

		m_model.SetPRS(m_pos, m_rot, m_model.GetScale());
		
	};

private:
	GameObj::CSkinModelRender m_model;
	AnimationClip m_animationClips[2];

	CVector3 m_pos = { 0.0f, 0.0f, 0.0f };
	CQuaternion m_rot;

	bool m_trigger = false;
};