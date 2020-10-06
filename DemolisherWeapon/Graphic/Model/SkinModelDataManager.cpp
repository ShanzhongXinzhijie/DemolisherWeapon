#include "DWstdafx.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "Skeleton.h"
#include "Model.h"

namespace DemolisherWeapon {

std::unique_ptr<DirectX::Model> SkinModelDataManager::CreateModel(const wchar_t* filePath, const Skeleton& skeleton)
{
#ifndef DW_DX12_TEMPORARY

	//ボーンを発見したときのコールバック関数。
	auto onFindBone = [&](
		const wchar_t* boneName,
		const VSD3DStarter::Bone* bone,
		std::vector<int>& localBoneIDtoGlobalBoneIDTbl
		)
	{
		int globalBoneID = skeleton.FindBoneID(boneName);
		if (globalBoneID == -1) {
			//ボーンが見つからなかった。
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "ボーンが見つかりません。\ntksファイルありますか?\n%ls\n", filePath);
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			return;
		}
		localBoneIDtoGlobalBoneIDTbl.push_back(globalBoneID);
	};

	//エフェクトファクトリ。
	SkinModelEffectFactory effectFactory(GetEngine().GetGraphicsEngine().GetD3DDevice());
	//テクスチャがあるフォルダを設定する。
	effectFactory.SetDirectory(L"Resource/modelData");

	//CMOファイルのロード。
#ifndef DW_MASTER
	try {
#endif
		std::unique_ptr<DirectX::Model> model = DirectX::Model::CreateFromCMO(	//CMOファイルからモデルを作成する関数の、CreateFromCMOを実行する。
			GetEngine().GetGraphicsEngine().GetD3DDevice(),			//第一引数はD3Dデバイス。
			filePath,									//第二引数は読み込むCMOファイルのファイルパス。
			effectFactory,								//第三引数はエフェクトファクトリ。
			false,										//第四引数はCullモード。今は気にしなくてよい。
			false,
			onFindBone
		);

		return model;

#ifndef DW_MASTER
	}
	catch (std::exception& exception) {
		// エラー処理
		char message[256];
		if (strcmp(exception.what(), "CreateFromCMO") == 0) {
			sprintf_s(message, "cmoファイルのロードに失敗。\nファイルパスあってますか?\n%ls\n", filePath);
		}
		else {
			sprintf_s(message, "cmoファイルのロードに失敗。\n%ls\n", filePath);
		}
		MessageBox(NULL, message, "Error", MB_OK);

		HRESULT hr = GetGraphicsEngine().GetD3DDevice()->GetDeviceRemovedReason();
		std::abort();
	}
#endif

#else
	return {};
#endif
}
std::unique_ptr<CModel> SkinModelDataManager::CreateCModel(const wchar_t* filePath, const Skeleton& skeleton) {
	//パスをcharへ変換...
	size_t iReturnValue;
	size_t size = wcslen(filePath) + 1;
	std::unique_ptr<char[]> charPath = std::make_unique<char[]>(size);
	errno_t err = wcstombs_s(
		&iReturnValue,
		charPath.get(),
		size, //上のサイズ
		filePath,
		size - 1 //コピーする最大文字数
	);
	if (err != 0) {
		DW_ERRORBOX(true, "wcstombs_s errno:%d", err)
	}

	//作成
	std::unique_ptr<CModel> model = std::make_unique<CModel>();
	model->LoadTkmFile(charPath.get());

	if (model->IsInited()) {
		model->CreateMeshParts();
		return model;
	}
	else {
		return {};
	}
}

DirectX::Model* SkinModelDataManager::Load(const wchar_t* filePath, const Skeleton& skeleton)
{
	DirectX::Model* retModel = NULL;//戻り値	

	//マップに登録されているか調べる。
	auto it = m_directXModelMap.find(filePath);
	if (it == m_directXModelMap.end()) {
		auto model = CreateModel(filePath, skeleton);
		//未登録なので、新規でロードする。
		retModel = model.get();
		//エラー
		if (!retModel) { return retModel; }
		//新規なのでマップに登録する。
		m_directXModelMap.insert({ filePath, std::move(model) });
	}
	else {
		//登録されているので、読み込み済みのデータを取得。
		retModel = it->second.get();
	}

	return retModel;
}
CModel* SkinModelDataManager::LoadCModel(const wchar_t* filePath, const Skeleton& skeleton)
{
	CModel* retModel = nullptr;//戻り値	

	//マップに登録されているか調べる。
	auto it = m_cmodelMap.find(filePath);
	if (it == m_cmodelMap.end()) {
		auto model = CreateCModel(filePath, skeleton);
		//未登録なので、新規でロードする。
		retModel = model.get();
		//エラー
		if (!retModel) { return retModel; }
		//新規なのでマップに登録する。
		m_cmodelMap.insert({ filePath, std::move(model) });
	}
	else {
		//登録されているので、読み込み済みのデータを取得。
		retModel = it->second.get();
	}

	return retModel;
}

void SkinModelDataManager::Release()
{
	//mapを空にする。
	m_directXModelMap.clear();
	m_cmodelMap.clear();
}

}