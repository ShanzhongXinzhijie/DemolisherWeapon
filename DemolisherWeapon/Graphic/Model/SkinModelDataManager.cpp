#include "DWstdafx.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "Skeleton.h"

namespace DemolisherWeapon {

DirectX::Model* SkinModelDataManager::Load(const wchar_t* filePath, const Skeleton& skeleton)
{
	DirectX::Model* retModel = NULL;
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
	//マップに登録されているか調べる。
	auto it = m_directXModelMap.find(filePath);
	if (it == m_directXModelMap.end()) {
		//未登録なので、新規でロードする。
		//エフェクトファクトリ。
		SkinModelEffectFactory effectFactory(GetEngine().GetGraphicsEngine().GetD3DDevice());
		//テクスチャがあるフォルダを設定する。
		effectFactory.SetDirectory(L"Resource/modelData");
		//CMOファイルのロード。
		try {
			auto model = DirectX::Model::CreateFromCMO(	//CMOファイルからモデルを作成する関数の、CreateFromCMOを実行する。
				GetEngine().GetGraphicsEngine().GetD3DDevice(),			//第一引数はD3Dデバイス。
				filePath,									//第二引数は読み込むCMOファイルのファイルパス。
				effectFactory,								//第三引数はエフェクトファクトリ。
				false,										//第四引数はCullモード。今は気にしなくてよい。
				false,
				onFindBone
			);		
			retModel = model.get();
			//新規なのでマップに登録する。
			m_directXModelMap.insert({ filePath, std::move(model) });
		}
		catch (std::exception& exception) {
			// エラー処理
#ifndef DW_MASTER
			char message[256];
			if (strcmp(exception.what() , "CreateFromCMO")==0) {
				sprintf_s(message, "cmoファイルのロードに失敗。\nファイルパスあってますか?\n%ls\n", filePath);
			}
			else {
				sprintf_s(message, "cmoファイルのロードに失敗。\n%ls\n", filePath);
			}
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
		}
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
}

}