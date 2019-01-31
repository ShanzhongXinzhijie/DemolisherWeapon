#pragma once
#include "system/GameObject.h"
#include "../animation/AnimationController.h"
#include "../animation/AnimationClip.h"

namespace DemolisherWeapon {
namespace GameObj {

class CSkinModelRender : public IQSGameObject{

public:
	CSkinModelRender();
	~CSkinModelRender();

private:
	void PreLoopUpdate()override final;
	void Update()override final;
	void PostUpdate()override final;
	void PostLoopUpdate()override final;	

	void ImNonUpdate() { m_isUpdated = false; };	
	void ImNonUpdateWorldMatrix() { m_isUpdatedWorldMatrix = false; };

public:

	//初期化
	void Init(const wchar_t* filePath,
		AnimationClip* animationClips = nullptr,
		int numAnimationClips = 0,
		EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
		EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded);

	void Init(const wchar_t* filePath, EnFbxUpAxis fbxUpAxis, EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded) {
		Init(filePath, nullptr, 0, fbxUpAxis, fbxCoordinate);
	};

	//座標・回転・拡大の設定
	void SetPos(const CVector3& pos) {
		ImNonUpdateWorldMatrix();
		m_pos = pos;
	}
	void SetRot(const CQuaternion& rot) {
		ImNonUpdateWorldMatrix();
		m_rot = rot;
	}
	void SetScale(const CVector3& scale) {
		ImNonUpdateWorldMatrix();
		m_scale = scale;
	}
	void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
		ImNonUpdateWorldMatrix();
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

	//ボーンの座標・回転・拡大の取得
	void CalcWorldTRS(int boneindex, CVector3& trans, CQuaternion& rot, CVector3& scale) {
		Update();
		GetSkinModel().GetSkeleton().GetBone(boneindex)->CalcWorldTRS(trans, rot, scale);		
	}
	const CVector3& GetBonePos(int boneindex) {
		Update();
		return GetSkinModel().GetSkeleton().GetBone(boneindex)->GetPosition();
	};
	const CQuaternion& GetBoneRot(int boneindex) {
		Update();
		return GetSkinModel().GetSkeleton().GetBone(boneindex)->GetRotation();
	};
	const CVector3& GetBoneScale(int boneindex) {
		Update();
		return GetSkinModel().GetSkeleton().GetBone(boneindex)->GetScale();
	};

	//ボーン番号を名前で検索
	int FindBoneID(const wchar_t* boneName) {
		return GetSkinModel().GetSkeleton().FindBoneID(boneName);
	}

	//ボーンのポインタを名前で検索
	Bone* FindBone(const wchar_t* boneName) {
		return GetSkinModel().FindBone(boneName);
	}

	//スキンモデルの取得
	SkinModel& GetSkinModel() {
		//ImNonUpdate();
		return m_model;
	}
	const SkinModel& GetSkinModel() const
	{
		return m_model;
	}

	//アニメーションコントローラーの取得
	AnimationController& GetAnimCon() {
		//ImNonUpdate();
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

	//一番奥に描画するか設定
	void SetIsMostDepth(bool enable) {
		if (enable) {
			m_model.SetRasterizerState(m_mostDepthRSCw, m_mostDepthRSCCw);
		}
		else {
			m_model.SetRasterizerState(nullptr, nullptr);
		}
	}

	//ワールド行列を更新
	void UpdateWorldMatrix() {
		m_model.UpdateWorldMatrix(m_pos, m_rot, m_scale);
		m_isUpdatedWorldMatrix = true;
	}

private:
	bool m_isInit = false;
	
	SkinModel m_model;
	CVector3		m_pos;
	CQuaternion		m_rot;
	CVector3		m_scale = CVector3::One();

	AnimationController m_animCon;

	bool m_isDraw = true;//表示するか
	bool m_isShadowCaster = true;//シャドウマップに書き込むか

	bool m_isUpdated = false;			//アップデート済みか?
	bool m_isUpdatedWorldMatrix = false;//ワールド行列更新済みか?

	static ID3D11RasterizerState* m_mostDepthRSCw;
	static ID3D11RasterizerState* m_mostDepthRSCCw;
};

}
}