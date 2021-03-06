#pragma once

#include "Skeleton.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "Util/Util.h"
#include "Render/MotionBlurRender.h"

namespace DemolisherWeapon {

class CModel;
struct SModelMesh;

/*!
*@brief	スキンモデルクラス。
*/
class SkinModel
{
public:
	//メッシュが見つかったときのコールバック関数。
	using OnFindMesh = std::function<void(const std::unique_ptr<DirectX::ModelMeshPart>&)>;
	using OnFindMaterial = std::function<void(ModelEffect*)>;

	/*!
	*@brief	デストラクタ。
	*/
	~SkinModel();
	
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="filePath">ロードするcmoまたはtkmファイルのファイルパス</param>
	/// <param name="enFbxUpAxis">fbxの上軸。デフォルトはenFbxUpAxisZ</param>
	/// <param name="enFbxCoordinate">fbxの座標系。デフォルトはenFbxRightHanded</param>
	/// <param name="isUseFlyweightFactory">モデルのロードにFlyweightFactoryを使用するか</param>
	void Init(std::filesystem::path filePath, EnFbxUpAxis enFbxUpAxis = enFbxUpAxisZ, EnFbxCoordinateSystem enFbxCoordinate = enFbxRightHanded, bool isUseFlyweightFactory = true);
	
	/*!
	*@brief	モデルをワールド座標系に変換するためのワールド行列を更新する。
			※スケルトンも更新される
	*@param[in]	position		モデルの座標。
	*@param[in]	rotation		モデルの回転。
	*@param[in]	scale			モデルの拡大率。
	*@param[in]	RefreshOldPos	旧行列をリフレッシュするか
	*/
	void UpdateWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, bool RefreshOldPos = false);
	//平行移動部分のみ更新
	void UpdateWorldMatrixTranslation(const CVector3& position, bool RefreshOldPos = false);

	/// <summary>
	/// ワールド行列を設定
	/// そして色々更新される
	/// </summary>
	/// <param name="worldMatrix">設定するワールド行列</param>
	/// <param name="RefreshOldPos">旧行列をリフレッシュするか</param>
	void SetWorldMatrix(const CMatrix& worldMatrix, bool RefreshOldPos = false);

	//ワールド行列を計算する
	void CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix, CMatrix& returnSRTMatrix)const;
	void CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix)const {
		CalcWorldMatrix(position, rotation, scale, returnWorldMatrix, returnWorldMatrix);
	}

	//(拡大×回転×平行移動)行列を計算する
	void CalcSRTMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix)const;

	//旧行列の記録
	void UpdateOldMatrix() {
		//前回のワールド行列を記録
		m_worldMatrixOld = m_worldMatrix;
		//ボーン
		m_skeleton.UpdateBoneMatrixOld();
	}

	/// <summary>
	/// ワールド行列の取得
	/// </summary>
	[[nodiscard]]
	const CMatrix& GetWorldMatrix()const {
		return m_worldMatrix;
	}

	/// <summary>
	/// バウンディングボックスを計算(ワールド行列を適応)
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="return_aabbMin">バウンディングボックスの最小値側が返ってくる</param>
	/// <param name="return_aabbMax">バウンディングボックスの最大値側が返ってくる</param>
	void CalcBoundingBoxWithWorldMatrix(const CMatrix& worldMatrix, CVector3& return_aabbMin, CVector3& return_aabbMax);

	/*!
	*@brief	ボーンを検索。
	*@param[in]		boneName	ボーンの名前。
	*@return	見つかったボーン。見つからなかった場合はnullptrを返します。
	*/
	[[nodiscard]]
	Bone* FindBone(const wchar_t* boneName)
	{
		int boneId = m_skeleton.FindBoneID(boneName);
		return m_skeleton.GetBone(boneId);
	}

	/*!
	*@brief	モデルを描画。
	*@param[in]	reverseCull  面を反転するか
	*@param[in]	instanceNum  インスタンス数
	*/
	void Draw(bool reverseCull = false, int instanceNum = 1, ID3D11BlendState* pBlendState = nullptr, ID3D11DepthStencilState* pDepthStencilState = nullptr);

	//モデルの名前を取得
	[[nodiscard]]
	const wchar_t* GetModelName()const
	{
		return m_modelName.c_str();
	}
	//モデルの名前の一致を判定
	[[nodiscard]]
	bool EqualModelName(const wchar_t* name) const
	{
		return wcscmp(name, m_modelName.c_str()) == 0;
	}

	/*!
	*@brief	スケルトンの取得。
	*/
	[[nodiscard]]
	Skeleton& GetSkeleton()
	{
		return m_skeleton;
	}

	/// <summary>
	/// メッシュを検索する。(DirectX::Model版)
	/// </summary>
	/// <param name="onFindMesh">メッシュが見つかったときのコールバック関数</param>
	void FindMesh(OnFindMesh onFindMesh) const
	{
#ifndef DW_DX12_TEMPORARY
		if (!m_modelDx) {
			DW_WARNING_BOX(true, "FindMesh:m_modelDxがNULL")
			return;
		}
		for (auto& modelMeshs : m_modelDx->meshes) {
			for (std::unique_ptr<DirectX::ModelMeshPart>& mesh : modelMeshs->meshParts) {
				onFindMesh(mesh);
			}
		}
#endif
	}
	/// <summary>
	/// メッシュを検索する。(CModel版)
	/// </summary>
	/// <param name="onFindMesh">メッシュが見つかったときのコールバック関数</param>
	void FindMeshCModel(std::function<void(const std::unique_ptr<SModelMesh>&)> onFindMesh) const;

	/// <summary>
	/// メッシュの集合を検索する。
	/// </summary>
	/// <param name="onFindMeshes">メッシュの集合が見つかったときのコールバック関数</param>
	void FindMeshes(std::function<void(const std::shared_ptr<DirectX::ModelMesh>&)> onFindMeshes)const 
	{
		if (!m_modelDx) {
			DW_WARNING_BOX(true, "FindMeshes:m_modelDxがNULL")
			return;
		}
		for (auto& modelMeshs : m_modelDx->meshes) {
			onFindMeshes(modelMeshs);
		}
	}

	/// <summary>
	/// マテリアルを検索する。(DirectX::Model版)
	/// </summary>
	/// <param name="onFindMaterial">マテリアルが見つかったときのコールバック関数</param>
	void FindMaterial(OnFindMaterial onFindMaterial) const
	{
#ifndef DW_DX12_TEMPORARY
		FindMesh([&](const std::unique_ptr<DirectX::ModelMeshPart>& mesh) {
			ModelEffect* effect = reinterpret_cast<ModelEffect*>(mesh->effect.get());
			onFindMaterial(effect);
		});
#endif
	}
	/// <summary>
	/// マテリアルデータを検索する
	/// </summary>
	/// <param name="onFindMaterial">マテリアルデータが見つかったときのコールバック関数</param>
	void FindMaterialData(std::function<void(MaterialData&)> onFindMaterial) const;

	//マテリアル設定を初期化して有効化
	void InitMaterialSetting() {
		isMatSetInit = true;
		isMatSetEnable = true;

		bool isAllocated = m_materialSetting.size() > 0;
		int i = 0;
		FindMaterialData(
			[&](MaterialData& mat) {
				if (!isAllocated) { m_materialSetting.emplace_back(); }//マテリアル設定の確保
				m_materialSetting[i].Init(&mat);//初期化
				i++;
			}
		);
	}
	//マテリアル設定の有効・無効を設定
	void SetMaterialSettingEnable(bool enable) {
		isMatSetEnable = enable;
	}
	//マテリアル設定を検索する
	void FindMaterialSetting(std::function<void(MaterialSetting*)> onFindMaterialSetting) 
	{
		if (!isMatSetInit) { InitMaterialSetting(); }

		for (auto& mat : m_materialSetting) {
			onFindMaterialSetting(&mat);
		}
	}



	//デプスステンシルステートをセット
	void SetDepthStencilState(ID3D11DepthStencilState* state) {
		m_pDepthStencilState = state;
	}
	//ラスタライザーステートをセット
	void SetRasterizerState(ID3D11RasterizerState* RSCw, ID3D11RasterizerState* RSCCw, ID3D11RasterizerState* RSNone) {
		m_pRasterizerStateCw = RSCw;
		m_pRasterizerStateCCw = RSCCw;
		m_pRasterizerStateNone = RSNone;
	}
	void GetRasterizerState(ID3D11RasterizerState*& RSCw, ID3D11RasterizerState*& RSCCw, ID3D11RasterizerState*& RSNone) const{
		RSCw = m_pRasterizerStateCw;
		RSCCw = m_pRasterizerStateCCw;
		RSNone = m_pRasterizerStateNone;
	}
	//深度バイアスをセット(シェーダ用)
	void SetDepthBias(float bias) {
		m_depthBias = bias;
	}
	float GetDepthBias()const {
		return m_depthBias;
	}

	//面の向きを設定
	void SetCullMode(D3D11_CULL_MODE cullMode) {
		m_cull = cullMode;
	}

	//シェーダー用変数tを設定
	void Set_t(float t) { m_cb_t = t; }

	/// <summary>
	/// インポスターの枚数を設定
	/// </summary>
	/// <param name="x">横の枚数</param>
	/// <param name="y">縦の枚数</param>
	void SetImposterPartNum(int x, int y) {
		m_imposterPartNum[0] = x;
		m_imposterPartNum[1] = y;
	}
	//インポスターのパラメータを設定
	void SetImposterParameter(float scale, float rotYrad) {
		m_imposterScale = scale;
		m_imposterRotY = rotYrad;
	}

	/// <summary>
	/// ソフトパーティクルが有効になる距離を設定
	/// </summary>
	void SetSoftParticleArea(float distance) {
		m_softParticleArea = distance;
	}

	//インスタンス数を設定
	void SetInstanceNum(int num) {
		m_instanceNum = num;
	}

	//ワールド行列を計算するか設定
	void SetIsCalcWorldMatrix(bool enable) {
		m_isCalcWorldMatrix = enable;
	}

	//視錐台カリングを行うか設定
	void SetIsFrustumCulling(bool enable) {
		m_isFrustumCull = enable;
	}
	bool GetIsFrustumCulling()const {
		return m_isFrustumCull;
	}

	/// <summary>
	/// バウンディングボックス(AABB)を設定
	/// </summary>
	/// <param name="min">ボックスの最小座標</param>
	/// <param name="max">ボックスの最大座標</param>
	void SetBoundingBox(const CVector3& min, const CVector3& max) {
		m_minAABB_Origin = min;
		m_maxAABB_Origin = max;
		m_centerAABB = m_minAABB_Origin + m_maxAABB_Origin; m_centerAABB /= 2.0f;
		m_extentsAABB = m_maxAABB_Origin - m_centerAABB;
		//バウンディングボックス初期化
		UpdateBoundingBoxWithWorldMatrix();
	}
	/// <summary>
	/// ワールド行列等を適応していないバウンディングボックス(AABB)を取得
	/// </summary>
	/// <param name="return_min">(戻り値)ボックスの最小座標</param>
	/// <param name="return_max">(戻り値)ボックスの最大座標</param>
	void GetBoundingBox(CVector3& return_min, CVector3& return_max)const {
		return_min = m_minAABB_Origin;
		return_max = m_maxAABB_Origin;
	}
	/// <summary>
	/// ワールド行列等適応したバウンディングボックス(AABB)を取得
	/// </summary>
	/// <param name="return_min">(戻り値)ボックスの最小座標</param>
	/// <param name="return_max">(戻り値)ボックスの最大座標</param>
	void GetUpdatedBoundingBox(CVector3& return_min, CVector3& return_max)const {
		return_min = m_minAABB;
		return_max = m_maxAABB;
	}
	/// <summary>
	/// モデル本来のバウンディングボックス(AABB)を取得
	/// </summary>
	/// <param name="return_center">ボックスの中心</param>
	/// <param name="return_extents">ボックスの中心から端までのベクトル</param>
	void GetModelOriginalBoundingBox(CVector3& return_center, CVector3& return_extents)const {
		return_center = m_modelBoxCenter, return_extents = m_modelBoxExtents;
	}

	//カリング前に行う処理を設定
	void SetPreCullingFunction(std::function<void(SkinModel*)> func) {
		m_preCullingFunc = func;
	}
	/// <summary>
	/// 描画前に行う処理を設定
	/// </summary>
	/// <param name="funcName">処理名</param>
	/// <param name="func">処理</param>
	/// <returns>設定に成功したか?(名前がかぶると追加できない)</returns>
	bool SetPreDrawFunction(const wchar_t* funcName, std::function<void(SkinModel*)> func) {
		return m_preDrawFunc.emplace(Util::MakeHash(funcName), func).second;
	}
	void ErasePreDrawFunction(const wchar_t* funcName) {
		m_preDrawFunc.erase(Util::MakeHash(funcName));
	}
	/// <summary>
	/// 描画後に行う処理を設定
	/// </summary>
	/// <param name="funcName">処理名</param>
	/// <param name="func">処理</param>
	/// <returns>設定に成功したか?(名前がかぶると追加できない)</returns>
	bool SetPostDrawFunction(const wchar_t* funcName, std::function<void(SkinModel*)> func) {
		return m_postDrawFunc.emplace(Util::MakeHash(funcName), func).second;
	}
	void ErasePostDrawFunction(const wchar_t* funcName) {
		m_postDrawFunc.erase(Util::MakeHash(funcName));
	}

	//描画を行うか設定
	void SetIsDraw(bool flag) { m_isDraw = flag; }
	bool GetIsDraw() const { return m_isDraw; }

	//FBXの設定取得
	[[nodiscard]]
	const EnFbxUpAxis& GetFBXUpAxis()const {
		return m_enFbxUpAxis;
	}
	[[nodiscard]]
	const EnFbxCoordinateSystem& GetFBXCoordinateSystem()const {
		return m_enFbxCoordinate;
	}

	/// <summary>
	/// DirectXTKのモデルをロード済みか?
	/// </summary>
	bool IsLoadedDirectXTKModel()const {
		return m_modelDx != nullptr;
	}
	/// <summary>
	/// モデルをロード済みか?
	/// </summary>
	bool IsLoadedModel()const {
		return m_model != nullptr;
	}

	/// <summary>
	/// モデルを取得
	/// </summary>
	/// <returns></returns>
	[[nodiscard]]
	CModel* GetModel() {
		return m_model;
	}

private:
	/*!
	*@brief	定数バッファの作成。
	*/
	void InitConstantBuffer();
	/*!
	*@brief	スケルトンの初期化。
	*@param[in]	filePath		ロードするcmoファイルのファイルパス。
	*/
	bool InitSkeleton(const wchar_t* filePath);	

	/// <summary>
	/// バウンディングボックスをワールド行列で更新
	/// </summary>
	void UpdateBoundingBoxWithWorldMatrix();

	/// <summary>
	/// ワールド行列更新後の処理
	/// </summary>
	/// <param name="isUpdatedWorldMatrix">ワールド行列が更新されたか?</param>
	/// <param name="RefreshOldPos">旧行列を更新するか?</param>
	void InnerUpdateWorldMatrix(bool isUpdatedWorldMatrix, bool RefreshOldPos);
	
private:
	//定数バッファ。
	//[model.fx:VSPSCb]
	struct SVSConstantBuffer {
		CMatrix mWorld;
		CMatrix mView;
		CMatrix mProj;

		//前回の行列
		CMatrix mWorld_old;
		CMatrix mView_old;
		CMatrix mProj_old;

		//カメラの移動量
		CVector4 camMoveVec;

		//深度値のバイアス
		CVector4 depthBias;

		//カメラのワールド座標
		CVector3 camWorldPos;

		//なんか
		float cb_t = 0.0f;

		//インポスター用
		int imposterPartNum[2];//分割数
		float imposterParameter[2];//x:スケール,y:Y軸回転
		
		//カメラのNear(x)・Far(y)
		CVector2 nearFar;

		//ソフトパーティクルが有効になる範囲
		float softParticleArea = 0.02f;

		//モーションブラースケール
		float MotionBlurScale = MotionBlurRender::DEFAULT_MBLUR_SCALE;
	};
	ConstantBuffer<SVSConstantBuffer> m_cb;//定数バッファ

	//行列
	CMatrix	m_worldMatrix;		//ワールド行列
	CMatrix m_worldMatrixOld;	//前回のワールド行列
	CMatrix m_biasMatrix;
	bool m_isFirstWorldMatRef = true;

	//シェーダー用設定	
	float m_cb_t = 0.0f;//シェーダー用なんか		
	int m_imposterPartNum[2] = {};//インポスター用分割数
	float m_imposterScale = 1.0f, m_imposterRotY = 0.0f;//インポスター用	
	float m_softParticleArea = 50.0f;//ソフトパーティクルが有効になる範囲

	//マテリアル個別設定	
	bool isMatSetInit = false;
	bool isMatSetEnable = false;
	std::vector<MaterialSetting> m_materialSetting;

	//モデルファイル情報
	EnFbxUpAxis			m_enFbxUpAxis = enFbxUpAxisZ;			//FBXの上方向
	EnFbxCoordinateSystem m_enFbxCoordinate = enFbxRightHanded;	//FBXの座標系
	std::wstring		m_modelName;							//モデルの名前

	//DirectXTKが提供するモデルクラス
	DirectX::Model* m_modelDx = nullptr;								
	std::unique_ptr<DirectX::Model> m_modelDxData;

	//このエンジンのモデルクラス
	CModel* m_model = nullptr;
	std::unique_ptr<CModel> m_modelData;

	//スケルトン
	Skeleton m_skeleton;

	//描画設定
	ID3D11DepthStencilState* m_pDepthStencilState = nullptr;//デプスステンシルステート
	ID3D11RasterizerState* m_pRasterizerStateCw = nullptr;//ラスタライザステート
	ID3D11RasterizerState* m_pRasterizerStateCCw = nullptr;
	ID3D11RasterizerState* m_pRasterizerStateNone = nullptr;
	float m_depthBias = 0.0f;//深度値バイアス	
	D3D11_CULL_MODE m_cull = D3D11_CULL_FRONT;//面の向き

	//インスタンス数
	int m_instanceNum = 1;

	//バウンディングボックス
	CVector3 m_minAABB_Origin, m_maxAABB_Origin;
	CVector3 m_centerAABB, m_extentsAABB;
	CVector3 m_minAABB, m_maxAABB;
	CVector3 m_modelBoxCenter, m_modelBoxExtents;

	//ユーザー設定の処理
	std::function<void(SkinModel*)> m_preCullingFunc = nullptr;//カリング前に実行
	std::unordered_map<int, std::function<void(SkinModel*)>> m_preDrawFunc;//描画前に実行
	std::unordered_map<int, std::function<void(SkinModel*)>> m_postDrawFunc;//描画後に実行

	//設定
	bool m_isDraw = true; //描画するか?
	bool m_isCalcWorldMatrix = true;//ワールド行列を計算するか?
	bool m_isFrustumCull = false;//視錐台カリングするか?

	//モデルデータマネージャー
	static SkinModelDataManager m_skinModelDataManager;
};

}