#include "DWstdafx.h"
#include "SkinModel.h"
#include "SkinModelShaderConst.h"
#include "Graphic/FrustumCulling.h"

#include <filesystem>

namespace DemolisherWeapon {

SkinModelDataManager SkinModel::m_skinModelDataManager;

SkinModel::~SkinModel()
{
	if (m_cb != nullptr) {
		//定数バッファを解放。
		m_cb->Release();
	}
}
void SkinModel::Init(const wchar_t* filePath, EnFbxUpAxis enFbxUpAxis, EnFbxCoordinateSystem enFbxCoordinate)
{
	//FBX情報を設定
	m_enFbxUpAxis = enFbxUpAxis;
	m_enFbxCoordinate = enFbxCoordinate;

	//バイアス行列取得
	CMatrix mBiasScr;
	CoordinateSystemBias::GetBias(m_biasMatrix, mBiasScr, m_enFbxUpAxis, m_enFbxCoordinate);
	m_biasMatrix.Mul(mBiasScr, m_biasMatrix);

	//スケルトンのデータを読み込む。
	bool hasSkeleton = InitSkeleton(filePath);

	//視錐台カリングする
	m_isFrustumCull = true;

	//定数バッファの作成。
	InitConstantBuffer();

	//SkinModelDataManagerを使用してCMOファイルのロード。
	m_modelDx = m_skinModelDataManager.Load(filePath, m_skeleton);	

	if (m_modelDx) {
		//マテリアル設定の確保
		FindMaterial(
			[&](ModelEffect* mat) {
			m_materialSetting.emplace_back();
		}
		);

		//バウンディングボックスの取得・生成
		bool isFirst = true;
		FindMeshes(
			[&](const std::shared_ptr<DirectX::ModelMesh>& meshes) {
				CVector3 size, extents;
				extents = meshes->boundingBox.Extents;

				//最大値
				size = meshes->boundingBox.Center;
				size += extents;
				if (isFirst) {
					m_maxAABB_Origin = size;
				}
				else {
					m_maxAABB_Origin.x = max(m_maxAABB_Origin.x, size.x);
					m_maxAABB_Origin.y = max(m_maxAABB_Origin.y, size.y);
					m_maxAABB_Origin.z = max(m_maxAABB_Origin.z, size.z);
				}
				//最小値
				size = meshes->boundingBox.Center;
				size -= extents;
				if (isFirst) {
					m_minAABB_Origin = size;
				}
				else {
					m_minAABB_Origin.x = min(m_minAABB_Origin.x, size.x);
					m_minAABB_Origin.y = min(m_minAABB_Origin.y, size.y);
					m_minAABB_Origin.z = min(m_minAABB_Origin.z, size.z);
				}

				isFirst = false;
			}
		);
		//中心と端までのベクトルを保存
		m_centerAABB = m_minAABB_Origin + m_maxAABB_Origin; m_centerAABB /= 2.0f;
		m_extentsAABB = m_maxAABB_Origin - m_centerAABB;
		//モデル本来のバウンディングボックスを保存
		m_modelBoxCenter = m_centerAABB, m_modelBoxExtents = m_extentsAABB;

		//スキンモデルなら
		if (hasSkeleton) {
			//大きさを二倍に(アニメーションしても収まるサイズ)(ホントに収まるかはしらん)
			CVector3 minBox = m_centerAABB - m_extentsAABB * 2.0f;
			CVector3 maxBox = m_centerAABB + m_extentsAABB * 2.0f;
			SetBoundingBox(minBox, maxBox);//設定
		}
	}

	//バウンディングボックス初期化
	UpdateBoundingBoxWithWorldMatrix();

	//ファイル名記録
	std::experimental::filesystem::path ps = filePath;
	m_modelName = ps.stem();
}
bool SkinModel::InitSkeleton(const wchar_t* filePath)
{
	//スケルトンのデータを読み込む。
	//cmoファイルの拡張子をtksに変更する。
	std::wstring skeletonFilePath = filePath;
	//文字列から.cmoファイル始まる場所を検索。
	int pos = (int)skeletonFilePath.find(L".cmo");
	if (pos == std::wstring::npos) {
#ifndef DW_MASTER
		char message[256];
		sprintf_s(message, "SkinModel::InitSkeleton\nCMOファイルじゃない!\n%ls\n", filePath);
		MessageBox(NULL, message, "Error", MB_OK);
		std::abort();
#endif
		return false;
	}
	//.cmoファイルを.tksに置き換える。
	skeletonFilePath.replace(pos, 4, L".tks");
	//tksファイルをロードする。
	bool result = m_skeleton.Load(skeletonFilePath.c_str());
	if ( result == false ) {
		//スケルトンが読み込みに失敗した。
		//アニメーションしないモデルは、スケルトンが不要なので
		//読み込みに失敗することはあるので、ログ出力だけにしておく。
#ifndef DW_MASTER
		char message[256];
		sprintf_s(message, "tksファイルの読み込みに失敗しました。%ls\n", skeletonFilePath.c_str());
		OutputDebugStringA(message);
#endif
		return false;
	}
	else {
		int numBone = m_skeleton.GetNumBones();
		for (int i = 0; i < numBone; i++) {			
			m_skeleton.GetBone(i)->SetCoordinateSystem(m_enFbxUpAxis, m_enFbxCoordinate);
		}
	}

	return true;
}
void SkinModel::InitConstantBuffer()
{
	//作成するバッファのサイズをsizeof演算子で求める。
	int bufferSize = sizeof(SVSConstantBuffer);
	//どんなバッファを作成するのかをせてbufferDescに設定する。
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));				//０でクリア。
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;						//バッファで想定されている、読み込みおよび書き込み方法。
	bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;	//バッファは16バイトアライメントになっている必要がある。
																//アライメントって→バッファのサイズが16の倍数ということです。
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;			//バッファをどのようなパイプラインにバインドするかを指定する。
																//定数バッファにバインドするので、D3D11_BIND_CONSTANT_BUFFERを指定する。
	bufferDesc.CPUAccessFlags = 0;								//CPU アクセスのフラグです。
																//CPUアクセスが不要な場合は0。
	//作成。
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_cb);
}

void SkinModel::UpdateWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, bool RefreshOldPos)
{
	if (m_isCalcWorldMatrix) {
		//ワールド行列を計算
		CalcWorldMatrix(position, rotation, scale, m_worldMatrix);
	}
	InnerUpdateWorldMatrix(m_isCalcWorldMatrix, RefreshOldPos);
}
void SkinModel::UpdateWorldMatrixTranslation(const CVector3& position, bool RefreshOldPos) 
{
	if (m_isCalcWorldMatrix) {
		//ワールド行列の平行移動部分を設定
		m_worldMatrix.SetTranslation(position);
	}
	InnerUpdateWorldMatrix(m_isCalcWorldMatrix, RefreshOldPos);
}

void SkinModel::SetWorldMatrix(const CMatrix& worldMatrix, bool RefreshOldPos) {
	m_worldMatrix = worldMatrix;
	InnerUpdateWorldMatrix(true, RefreshOldPos);
}

void SkinModel::InnerUpdateWorldMatrix(bool isUpdatedWorldMatrix, bool RefreshOldPos) {
	if (isUpdatedWorldMatrix) {
		//スケルトンの更新。
		m_skeleton.Update(m_worldMatrix);
		//バウンディングボックスを更新
		UpdateBoundingBoxWithWorldMatrix();
	}
	else {
		//スケルトンの更新。
		m_skeleton.Update(CMatrix::Identity());
	}

	//最初のワールド座標更新なら...
	if (m_isFirstWorldMatRef || RefreshOldPos) {
		m_isFirstWorldMatRef = false;
		//旧座標の更新
		UpdateOldMatrix();
	}
}

void SkinModel::CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix, CMatrix& returnSRTMatrix)const {
	//(拡大×回転×平行移動)行列を適応
	CalcSRTMatrix(position, rotation, scale, returnSRTMatrix);
	returnWorldMatrix = returnSRTMatrix;

	//バイアス適応
	returnWorldMatrix.Mul(m_biasMatrix, returnWorldMatrix);
}

void SkinModel::CalcSRTMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix)const {
	//(拡大×回転×平行移動)行列を作成
	CMatrix mat;
	returnWorldMatrix.MakeScaling(scale);			//拡大	
	mat.MakeRotationFromQuaternion(rotation);		//回転	
	returnWorldMatrix.Mul(returnWorldMatrix, mat);	//拡大×回転
	returnWorldMatrix.SetTranslation(position);		//平行移動を設定
}

void SkinModel::CalcBoundingBoxWithWorldMatrix(const CMatrix& worldMatrix, CVector3& return_aabbMin, CVector3& return_aabbMax) {
	static const CVector3 boxOffset[8] =
	{
		{ -1.0f, -1.0f,  1.0f },
		{  1.0f, -1.0f,  1.0f },
		{  1.0f,  1.0f,  1.0f },
		{ -1.0f,  1.0f,  1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{  1.0f, -1.0f, -1.0f },
		{  1.0f,  1.0f, -1.0f },
		{ -1.0f,  1.0f, -1.0f },
	};

	//AABBを作る
	CVector3 vertex;
	for (int i = 0; i < 8; i++) {
		vertex = m_centerAABB + m_extentsAABB * boxOffset[i];
		worldMatrix.Mul(vertex);
		if (i == 0) {
			return_aabbMin = vertex; return_aabbMax = vertex;
		}
		else {
			return_aabbMin.x = min(return_aabbMin.x, vertex.x); return_aabbMin.y = min(return_aabbMin.y, vertex.y); return_aabbMin.z = min(return_aabbMin.z, vertex.z);
			return_aabbMax.x = max(return_aabbMax.x, vertex.x); return_aabbMax.y = max(return_aabbMax.y, vertex.y); return_aabbMax.z = max(return_aabbMax.z, vertex.z);
		}
	}
}
void SkinModel::UpdateBoundingBoxWithWorldMatrix(){
	CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);
}

static const float REFERENCE_FRUSTUM_SIZE = (1.0f / tan(3.14f*0.5f / 2.0f));

void SkinModel::Draw(bool reverseCull, int instanceNum, ID3D11BlendState* pBlendState, ID3D11DepthStencilState* pDepthStencilState)
{
#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "カメラが設定されていません!!", "Error", MB_OK);
		std::abort();
		return;
	}
#endif

	//描画フラグ立っているか
	//if (!GetIsDraw()) { return; }

	//ユーザー設定のカリング前処理実行
	if (m_preCullingFunc) { m_preCullingFunc(this); }

	//描画インスタンス数が0
	if (instanceNum*m_instanceNum <= 0) { return; }

	//視錐台カリング
	if (m_isFrustumCull) {
		if (!FrustumCulling::AABBTest(GetMainCamera(), m_minAABB, m_maxAABB)) {
			return;
		}
	}

	ID3D11DeviceContext* d3dDeviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//定数バッファの内容を更新。
	{
		SVSConstantBuffer vsCb;
		vsCb.mWorld = m_worldMatrix;
		vsCb.mProj = GetMainCamera()->GetProjMatrix();
		vsCb.mView = GetMainCamera()->GetViewMatrix();

		vsCb.mWorld_old = m_worldMatrixOld;
		vsCb.mProj_old = GetMainCamera()->GetProjMatrixOld();
		vsCb.mView_old = GetMainCamera()->GetViewMatrixOld();

		vsCb.camMoveVec = (GetMainCamera()->GetPos() - GetMainCamera()->GetPosOld())*MotionBlurScale;
		vsCb.camMoveVec.w = GetEngine().GetDistanceScale();

		vsCb.depthBias.x = m_depthBias;
		vsCb.depthBias.y = (GetMainCamera()->GetFar() - GetMainCamera()->GetNear())*vsCb.depthBias.x;
		vsCb.depthBias.z = 50.0f*GetEngine().GetDistanceScale()*(GetMainCamera()->GetProjMatrix().m[1][1] / REFERENCE_FRUSTUM_SIZE);

		vsCb.camWorldPos = GetMainCamera()->GetPos();

		vsCb.cb_t = m_cb_t;

		//インポスター分割数
		vsCb.imposterPartNum[0] = m_imposterPartNum[0];
		vsCb.imposterPartNum[1] = m_imposterPartNum[1];
		//インポスターParameter
		vsCb.imposterParameter[0] = m_imposterScale;//スケール
		vsCb.imposterParameter[1] = m_imposterRotY;//Y軸回転

		//カメラのNear・Far
		vsCb.nearFar.x = GetMainCamera()->GetNear();
		vsCb.nearFar.y = GetMainCamera()->GetFar();

		//ソフトパーティクルが有効になる範囲
		vsCb.softParticleArea = m_softParticleArea;

		//定数バッファ更新
		d3dDeviceContext->UpdateSubresource(m_cb, 0, nullptr, &vsCb, 0, 0);
	}

	//定数バッファをGPUに転送。
	d3dDeviceContext->VSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	d3dDeviceContext->PSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	//ボーン行列をGPUに転送。
	m_skeleton.SendBoneMatrixArrayToGPU();

	//マテリアル設定の適応
	if(isMatSetInit && isMatSetEnable){
		//個別設定
		int i = 0;
		FindMaterial(
			[&](ModelEffect* mat) {
				mat->SetUseMaterialSetting(m_materialSetting[i]);
				i++;
			}
		);
	}
	else {
		//全体設定
		FindMaterial([&](ModelEffect* mat) { mat->SetDefaultMaterialSetting(); });
	}

	//ユーザー設定の描画前処理実行
	for (auto& func : m_preDrawFunc) {
		func.second(this);
	}

	//面の向き
	D3D11_CULL_MODE cullMode = m_cull;
	if ((m_enFbxCoordinate == enFbxRightHanded) != reverseCull) {
		//反転
		if (cullMode == D3D11_CULL_FRONT) { 
			cullMode = D3D11_CULL_BACK; 
		}else
		if (cullMode == D3D11_CULL_BACK) { 
			cullMode = D3D11_CULL_FRONT; 
		}
	}

	//描画。
	m_modelDx->Draw(
		d3dDeviceContext,
		GetGraphicsEngine().GetCommonStates(),
		m_worldMatrix,
		GetMainCamera()->GetViewMatrix(),
		GetMainCamera()->GetProjMatrix(),
		false,
		cullMode,
		pBlendState,
		m_pRasterizerStateCw, m_pRasterizerStateCCw, m_pRasterizerStateNone,
		pDepthStencilState,
		instanceNum*m_instanceNum
	);

	//ユーザー設定の描画後処理実行
	for (auto& func : m_postDrawFunc) {
		func.second(this);
	}
}

}