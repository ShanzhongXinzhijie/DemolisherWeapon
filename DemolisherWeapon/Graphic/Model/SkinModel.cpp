#include "DWstdafx.h"
#include "SkinModel.h"
#include "SkinModelShaderConst.h"

#include <filesystem>

namespace DemolisherWeapon {

SkinModelDataManager SkinModel::m_skinModelDataManager;

SkinModel::~SkinModel()
{
	if (m_cb != nullptr) {
		//定数バッファを解放。
		m_cb->Release();
	}
	if (m_samplerState != nullptr) {
		//サンプラステートを解放。
		m_samplerState->Release();
	}
}
void SkinModel::Init(const wchar_t* filePath, EnFbxUpAxis enFbxUpAxis, EnFbxCoordinateSystem enFbxCoordinate)
{
	m_enFbxUpAxis = enFbxUpAxis;
	m_enFbxCoordinate = enFbxCoordinate;

	//スケルトンのデータを読み込む。
	InitSkeleton(filePath);

	//定数バッファの作成。
	InitConstantBuffer();

	//サンプラステートの初期化。
	InitSamplerState();

	//SkinModelDataManagerを使用してCMOファイルのロード。
	m_modelDx = m_skinModelDataManager.Load(filePath, m_skeleton);	

	//ファイル名記録
	std::experimental::filesystem::path ps = filePath;
	m_modelName = ps.stem();
}
void SkinModel::InitSkeleton(const wchar_t* filePath)
{
	//スケルトンのデータを読み込む。
	//cmoファイルの拡張子をtksに変更する。
	std::wstring skeletonFilePath = filePath;
	//文字列から.cmoファイル始まる場所を検索。
	int pos = (int)skeletonFilePath.find(L".cmo");
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
	}
	else {
		int numBone = m_skeleton.GetNumBones();
		for (int i = 0; i < numBone; i++) {			
			m_skeleton.GetBone(i)->SetCoordinateSystem(m_enFbxUpAxis, m_enFbxCoordinate);
		}
	}
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
void SkinModel::InitSamplerState()
{
	//テクスチャのサンプリング方法を指定するためのサンプラステートを作成。
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
}
void SkinModel::UpdateWorldMatrix(CVector3 position, CQuaternion rotation, CVector3 scale)
{
	CMatrix mBiasRot;
	CMatrix mBiasScr;

	CoordinateSystemBias::GetBias(mBiasRot, mBiasScr, m_enFbxUpAxis, m_enFbxCoordinate);

	CMatrix transMatrix, rotMatrix, scaleMatrix;
	//平行移動行列を作成する。
	transMatrix.MakeTranslation( position );
	//回転行列を作成する。
	rotMatrix.MakeRotationFromQuaternion( rotation );
	//rotMatrix.Mul(mBiasRot, rotMatrix);
	//拡大行列を作成する。
	scaleMatrix.MakeScaling(scale);
	//scaleMatrix.Mul(mBiasScr, scaleMatrix);

	//ワールド行列を作成する。
	//拡大×回転×平行移動の順番で乗算するように！
	//順番を間違えたら結果が変わるよ。
	m_worldMatrix.Mul(scaleMatrix, rotMatrix);
	m_worldMatrix.Mul(m_worldMatrix, transMatrix);

	//バイアス適応
	mBiasRot.Mul(mBiasScr, mBiasRot);
	m_worldMatrix.Mul(mBiasRot, m_worldMatrix);

	//スケルトンの更新。
	m_skeleton.Update(m_worldMatrix);

	//最初のワールド座標更新なら...
	if (m_isFirstWorldMatRef) {
		m_isFirstWorldMatRef = false;
		//旧座標の更新
		UpdateOldMatrix();
	}
}
void SkinModel::Draw(bool reverseCull)
{
	DirectX::CommonStates state(GetEngine().GetGraphicsEngine().GetD3DDevice());

	ID3D11DeviceContext* d3dDeviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//定数バッファの内容を更新。
	SVSConstantBuffer vsCb;
	vsCb.mWorld = m_worldMatrix;
	vsCb.mProj = GetMainCamera()->GetProjMatrix();
	vsCb.mView = GetMainCamera()->GetViewMatrix();

	vsCb.mWorld_old = m_worldMatrixOld;
	vsCb.mProj_old = GetMainCamera()->GetProjMatrixOld();
	vsCb.mView_old = GetMainCamera()->GetViewMatrixOld();
	
	vsCb.isMotionBlur = m_isMotionBlur ? 1 : 0;

	vsCb.alignment[0] = vsCb.alignment[1] = vsCb.alignment[2] = 0;

	d3dDeviceContext->UpdateSubresource(m_cb, 0, nullptr, &vsCb, 0, 0);

	//定数バッファをGPUに転送。
	d3dDeviceContext->VSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	d3dDeviceContext->PSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	//サンプラステートを設定。
	d3dDeviceContext->PSSetSamplers(0, 1, &m_samplerState);
	//ボーン行列をGPUに転送。
	m_skeleton.SendBoneMatrixArrayToGPU();

	//描画。
	m_modelDx->Draw(
		d3dDeviceContext,
		state,
		m_worldMatrix,
		GetMainCamera()->GetViewMatrix(),
		GetMainCamera()->GetProjMatrix(),
		false,
		(m_enFbxCoordinate == enFbxRightHanded)!=reverseCull
	);

	//旧座標の更新
	//UpdateOldMatrix();
}

}