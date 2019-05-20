#pragma once

#include "Skeleton.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "../Render/MotionBlurRender.h"

namespace DemolisherWeapon {

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
	
	/*!
	*@brief	初期化。
	*@param[in]	filePath		ロードするcmoファイルのファイルパス。
	*@param[in] enFbxUpAxis		fbxの上軸。デフォルトはenFbxUpAxisZ。
	*/
	void Init(const wchar_t* filePath, EnFbxUpAxis enFbxUpAxis = enFbxUpAxisZ, EnFbxCoordinateSystem enFbxCoordinate = enFbxRightHanded);
	
	/*!
	*@brief	モデルをワールド座標系に変換するためのワールド行列を更新する。
	*@param[in]	position	モデルの座標。
	*@param[in]	rotation	モデルの回転。
	*@param[in]	scale		モデルの拡大率。
	*/
	void UpdateWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, bool RefreshOldPos = false);

	//ワールド行列を計算する
	void CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix);

	//旧行列の記録
	void UpdateOldMatrix() {
		//前回のワールド行列を記録
		m_worldMatrixOld = m_worldMatrix;
		//ボーン
		m_skeleton.UpdateBoneMatrixOld();
	}

	/*!
	*@brief	ボーンを検索。
	*@param[in]		boneName	ボーンの名前。
	*@return	見つかったボーン。見つからなかった場合はnullptrを返します。
	*/
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
	const wchar_t* GetModelName()const
	{
		return m_modelName.c_str();
	}
	//モデルの名前の一致を判定
	bool EqualModelName(const wchar_t* name) const
	{
		return wcscmp(name, m_modelName.c_str()) == 0;
	}

	/*!
	*@brief	スケルトンの取得。
	*/
	Skeleton& GetSkeleton()
	{
		return m_skeleton;
	}

	/*!
	*@brief	メッシュを検索する。
	*@param[in] onFindMesh		メッシュが見つかったときのコールバック関数
	*/
	void FindMesh(OnFindMesh onFindMesh) const
	{
		for (auto& modelMeshs : m_modelDx->meshes) {
			for (std::unique_ptr<DirectX::ModelMeshPart>& mesh : modelMeshs->meshParts) {
				onFindMesh(mesh);
			}
		}
	}
	/*!
	*@brief	マテリアルを検索する。
	*@param[in] onFindMaterial	マテリアルが見つかったときのコールバック関数
	*/
	void FindMaterial(OnFindMaterial onFindMaterial) const
	{
		FindMesh([&](auto& mesh) {
			ModelEffect* effect = reinterpret_cast<ModelEffect*>(mesh->effect.get());
			onFindMaterial(effect);
		});
	}

	//マテリアル設定を初期化して有効化
	void InitMaterialSetting() {
		isMatSetInit = true;
		isMatSetEnable = true;

		int i = 0;
		FindMaterial(
			[&](ModelEffect* mat) {
				mat->MaterialSettingInit(m_materialSetting[i]);
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

	//ラスタライザーステートをセット
	void SetRasterizerState(ID3D11RasterizerState* RSCw, ID3D11RasterizerState* RSCCw) {
		m_pRasterizerStateCw = RSCw;
		m_pRasterizerStateCCw = RSCCw;
	}
	void GetRasterizerState(ID3D11RasterizerState*& RSCw, ID3D11RasterizerState*& RSCCw) const{
		RSCw = m_pRasterizerStateCw;
		RSCCw = m_pRasterizerStateCCw;
	}
	//深度バイアスをセット(シェーダ用)
	void SetDepthBias(float bias) {
		m_depthBias = bias;
	}

	//インスタンス数を設定
	void SetInstanceNum(int num) {
		m_instanceNum = num;
	}

	//ワールド行列を計算するか設定
	void SetIsCalcWorldMatrix(bool enable) {
		m_isCalcWorldMatrix = enable;
	}

	//描画前に行う処理を設定
	void SetPreDrawFunction(std::function<void(SkinModel*)> func) {
		m_preDrawFunc = func;
	}

	//FBXの設定取得
	const EnFbxUpAxis& GetFBXUpAxis()const {
		return m_enFbxUpAxis;
	}
	const EnFbxCoordinateSystem& GetFBXCoordinateSystem()const {
		return m_enFbxCoordinate;
	}

private:
	/*!
	*@brief	サンプラステートの初期化。
	*/
	//void InitSamplerState();
	/*!
	*@brief	定数バッファの作成。
	*/
	void InitConstantBuffer();
	/*!
	*@brief	スケルトンの初期化。
	*@param[in]	filePath		ロードするcmoファイルのファイルパス。
	*/
	void InitSkeleton(const wchar_t* filePath);	
	
private:
	//定数バッファ。
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

		CVector4 depthBias;

		//カメラのワールド座標
		CVector3 camWorldPos;
	};
	CMatrix	m_worldMatrix;		//ワールド行列
	CMatrix m_worldMatrixOld;	//前回のワールド行列
	bool m_isFirstWorldMatRef = true;

	//マテリアル個別設定	
	bool isMatSetInit = false;
	bool isMatSetEnable = false;
	std::vector<MaterialSetting> m_materialSetting;

	EnFbxUpAxis			m_enFbxUpAxis = enFbxUpAxisZ;	//!<FBXの上方向。
	EnFbxCoordinateSystem m_enFbxCoordinate = enFbxRightHanded;//FBXの座標系
	ID3D11Buffer*		m_cb = nullptr;					//!<定数バッファ。
	Skeleton			m_skeleton;						//!<スケルトン。
	DirectX::Model*		m_modelDx;						//!<DirectXTKが提供するモデルクラス。
	std::wstring		m_modelName;					//!<モデルの名前。
	//ID3D11SamplerState* m_samplerState = nullptr;		//!<サンプラステート。
	ID3D11RasterizerState* m_pRasterizerStateCw = nullptr;
	ID3D11RasterizerState* m_pRasterizerStateCCw = nullptr;
	float m_depthBias = 0.0f;

	int m_instanceNum = 1;//インスタンス数

	bool m_isCalcWorldMatrix = true;//ワールド行列を計算するか?

	std::function<void(SkinModel*)> m_preDrawFunc = nullptr;//ユーザー設定の処理

	static SkinModelDataManager m_skinModelDataManager;
};

}