#pragma once
#include "system/GameObject.h"
#include "../animation/AnimationController.h"
#include "../animation/AnimationClip.h"

namespace DemolisherWeapon {
namespace GameObj {

class CSkinModelRender : public IQSGameObject{

public:
	CSkinModelRender();

private:
	void PreLoopUpdate()override final;
	void Update()override final;
	void PostUpdate()override final;
	void PostLoopUpdate()override final;	

	void ImNonUpdate() { m_isUpdated = false; };	
	void ImNonUpdateWorldMatrix() { m_isUpdatedWorldMatrix = false; };

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <param name="animationClips">アニメーションクリップ配列</param>
	/// <param name="numAnimationClips">アニメーションクリップ数</param>
	/// <param name="fbxUpAxis">fbxの上軸。デフォルトはenFbxUpAxisZ</param>
	/// <param name="fbxCoordinate">fbxの座標系。デフォルトはenFbxRightHanded</param>
	/// <param name="isUseFlyweightFactory">モデルのロードにFlyweightFactoryを使用するか</param>
	void Init(const wchar_t* filePath,
		AnimationClip* animationClips = nullptr,
		int numAnimationClips = 0,
		EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
		EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,
		bool isUseFlyweightFactory = true);

	void Init(const wchar_t* filePath, EnFbxUpAxis fbxUpAxis, EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded, bool isUseFlyweightFactory = true) {
		Init(filePath, nullptr, 0, fbxUpAxis, fbxCoordinate, isUseFlyweightFactory);
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
	void SetIsDraw(bool flag) { m_model.SetIsDraw(flag); }
	bool GetIsDraw() const { return m_model.GetIsDraw(); }

	//面を反転させるか設定
	void SetIsDrawReverse(bool flag) { m_isDrawReverse = flag; }
	bool GetIsDrawReverse()const { return m_isDrawReverse; }

	/// <summary>
	/// ソフトパーティクルの種類
	/// </summary>
	enum enSoftParticleMode{
		enOff,//オフ
		enSoftParticle,//通常
		enRevSoftParticle,//逆
	};
	/// <summary>
	/// ポストドロー描画を初期化
	/// </summary>
	/// <param name="blendMode">ブレンドモード</param>
	/// <param name="isPMA">モデルが乗算済みアルファか?</param>
	/// <param name="isSoftParticle">ソフトパーティクルを有効化するか</param>
	/// <param name="softParticleArea">ソフトパーティクルが発生する距離範囲(＜0でデフォルト)</param>
	void InitPostDraw(PostDrawModelRender::enBlendMode blendMode, bool isPMA = false, enSoftParticleMode isSoftParticle = enOff, float softParticleArea = -1.0f);

	//シャドウマップへの描画を行うか設定
	void SetIsShadowCaster(bool flag) { m_isShadowCaster = flag; }
	//シャドウマップの描画時に面を反転させるか設定
	void SetIsShadowDrawReverse(bool flag) { m_isShadowDrawReverse = flag; }
	bool GetIsShadowDrawReverse()const { return m_isShadowDrawReverse; }

	//バウンディングボックスを表示するか設定
	void SetIsDrawBoundingBox(bool enable) { m_isDrawBoundingBox = enable; }

	//一番奥に描画するか設定
	void SetIsMostDepth(bool enable) {
		if (enable) {
			m_model.SetRasterizerState(m_mostDepthRSCw, m_mostDepthRSCCw, m_mostDepthRSNone);
			m_model.SetDepthBias(1.0f);
		}
		else {
			m_model.SetRasterizerState(nullptr, nullptr, nullptr);
			m_model.SetDepthBias(0.0f);
		}
	}

	//描画順を設定(数値が大きいほど後に描画される)
	void SetDrawPriority(int prio) { m_priority = prio; }
	int GetDrawPriority()const { return m_priority; }

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

	//bool m_isDraw = true;				//表示するか
	bool m_isDrawReverse = false;		//面を反転して描画するか
	bool m_isShadowCaster = true;		//シャドウマップに書き込むか
	bool m_isShadowDrawReverse = true;	//シャドウマップ描画時に面を反転させるか?
	bool m_isDrawBoundingBox = false;	//バウンディングボックスを表示するか?
	bool m_isPostDraw = false;			//ポストドローか?
	PostDrawModelRender::enBlendMode m_postDrawBlendMode = PostDrawModelRender::enBlendMode::enAlpha;//ポストドローのブレンドモード

	bool m_isUpdated = false;			//アップデート済みか?
	bool m_isUpdatedWorldMatrix = false;//ワールド行列更新済みか?
	bool m_isSetRotOrScale = true;		//回転または拡大を設定したか?

	bool m_animUpdating = false;

	//素材の味シェーダ
	Shader m_psSozaiNoAzi, m_psSozaiNoAziTex;
	bool m_loadedShaderSNA = false;			//素材の味シェーダはロード済みか?
	bool m_shaderSNAIsConvertPMA = false;	//素材の味シェーダは乗算済みアルファ変換版か?
	enSoftParticleMode m_shaderSNAIsSoftParticle = enOff;	//素材の味シェーダはソフトパーティクル版か?

	//std::unique_ptr<ShadowMapRender::IPrePost> m_shadowMapPrePost;

	static ID3D11RasterizerState* m_mostDepthRSCw;
	static ID3D11RasterizerState* m_mostDepthRSCCw;
	static ID3D11RasterizerState* m_mostDepthRSNone;
};

}
}