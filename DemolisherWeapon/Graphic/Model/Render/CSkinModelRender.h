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
		ImNonUpdateWorldMatrix(); m_isSetRotOrScale = true;
		m_rot = rot;
	}
	void SetScale(const CVector3& scale) {
		ImNonUpdateWorldMatrix(); m_isSetRotOrScale = true;
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

	//Update関数を実行するか設定
	void SetIsEnableUpdate(bool enable) {
		m_isEnableUpdate = enable;
	}

	//描画を行うか設定
	void SetIsDraw(bool flag) { m_isDraw = flag; }
	bool GetIsDraw() const { return m_isDraw; }

	//シャドウマップへの描画を行うか設定
	void SetIsShadowCaster(bool flag) { m_isShadowCaster = flag; }
	//シャドウマップの描画時に面を反転させるか設定
	void SetIsShadowDrawReverse(bool flag) { m_isShadowDrawReverse = flag; }
	//シャドウマップ描画前後で行う処理を設定
	/*void SetShadowMapPrePost(std::unique_ptr<ShadowMapRender::IPrePost>&& prepost) {
		m_shadowMapPrePost = std::move(prepost);
	}
	ShadowMapRender::IPrePost* GetShadowMapPrePost()const {
		return m_shadowMapPrePost.get();
	}*/

	//バウンディングボックスを表示するか設定
	void SetIsDrawBoundingBox(bool enable) { m_isDrawBoundingBox = enable; }

	//一番奥に描画するか設定
	void SetIsMostDepth(bool enable) {
		if (enable) {
			m_model.SetRasterizerState(m_mostDepthRSCw, m_mostDepthRSCCw);
			m_model.SetDepthBias(1.0f);
		}
		else {
			m_model.SetRasterizerState(nullptr, nullptr);
			m_model.SetDepthBias(0.0f);
		}
	}

	//描画順を設定(数値が大きいほど後に描画される)
	void SetDrawPriority(int prio) {
		m_priority = prio;
	}
	int GetDrawPriority()const {
		return m_priority;
	}

	//ワールド行列を更新(スケルトンも更新される)
	void UpdateWorldMatrix(bool refreshOldPos = false) {
		if (m_isSetRotOrScale) {
			//ワールド行列更新
			m_model.UpdateWorldMatrix(m_pos, m_rot, m_scale, refreshOldPos);
			m_isSetRotOrScale = false;
		}
		else {
			//ワールド行列の平行移動部分更新
			m_model.UpdateWorldMatrixTranslation(m_pos, refreshOldPos);
		}
		m_isUpdatedWorldMatrix = true;
	}
	//ワールド行列をリフレッシュ
	void RefreshWorldMatrix() {
		ImNonUpdate();
		ImNonUpdateWorldMatrix(); m_isSetRotOrScale = true;
		m_isRefreshMode = true;
		Update();
		m_isRefreshMode = false;
	}

private:
	bool m_isInit = false;
	bool m_isRefreshMode = false;

	int m_priority = DRAW_PRIORITY_DEFAULT;//描画順
	
	SkinModel m_model;
	CVector3		m_pos;
	CQuaternion		m_rot;
	CVector3		m_scale = CVector3::One();

	AnimationController m_animCon;

	bool m_isEnableUpdate = true;//Updateを実行するか?

	bool m_isDraw = true;				//表示するか
	bool m_isShadowCaster = true;		//シャドウマップに書き込むか
	bool m_isShadowDrawReverse = true;	//シャドウマップ描画時に面を反転させるか?
	bool m_isDrawBoundingBox = false;	//バウンディングボックスを表示するか?

	bool m_isUpdated = false;			//アップデート済みか?
	bool m_isUpdatedWorldMatrix = false;//ワールド行列更新済みか?
	bool m_isSetRotOrScale = true;		//回転または拡大を設定したか?

	bool m_animUpdating = false;

	//std::unique_ptr<ShadowMapRender::IPrePost> m_shadowMapPrePost;

	static ID3D11RasterizerState* m_mostDepthRSCw;
	static ID3D11RasterizerState* m_mostDepthRSCCw;
};

}
}