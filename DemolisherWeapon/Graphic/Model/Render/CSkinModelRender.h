#pragma once
#include "system/GameObject.h"
#include "../animation/AnimationController.h"
#include "../animation/AnimationClip.h"

namespace DemolisherWeapon {
namespace GameObj {

class CSkinModelRender : public IGameObject{

public:
	CSkinModelRender();
	~CSkinModelRender();

private:
	void Update()override final;
	void PostLoopUpdate()override final;	
public:

	//初期化
	void Init(const wchar_t* filePath,
		AnimationClip* animationClips = nullptr,
		int numAnimationClips = 0,
		EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ);

	//座標・回転・拡大の設定
	void SetPos(const CVector3& pos) {
		m_pos = pos;
	}
	void SetRot(const CQuaternion& rot) {
		m_rot = rot;
	}
	void SetScale(const CVector3& scale) {
		m_scale = scale;
	}
	void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
		SetPos(pos);
		SetRot(rot);
		SetScale(scale);
	}
	//座標・回転・拡大の取得
	const CVector3& GetPos() const{
		return m_pos ;
	}
	const CQuaternion& GetRot() const {
		return m_rot;
	}
	const CVector3& GetScale() const {
		return m_scale;
	}
	void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const{
		*pos = GetPos();
		*rot = GetRot();
		*scale = GetScale();
	}

	//スキンモデルの取得
	SkinModel& GetSkinModel() {
		return m_model;
	}
	const SkinModel& GetSkinModel() const
	{
		return m_model;
	}

	//アニメーションコントローラーの取得
	AnimationController& GetAnimCon() {
		return m_animCon;
	}	

	//描画を行うか設定
	void SetIsDraw(bool flag) {
		m_isDraw = flag;
	}
	//シャドウマップへの描画を行うか設定
	void SetIsShadowCaster(bool flag){
		m_isShadowCaster = flag;
	}

	//ワールド行列を更新
	void UpdateWorldMatrix() {
		m_model.UpdateWorldMatrix(m_pos, m_rot, m_scale);
	}

private:
	SkinModel m_model;
	CVector3		m_pos;
	CQuaternion		m_rot;
	CVector3		m_scale = CVector3::One();

	AnimationController m_animCon;

	bool m_isDraw = true;//表示するか
	bool m_isShadowCaster = true;//シャドウマップに書き込むか
};

}
}