#include "DWstdafx.h"
#include "Level.h"
#include "MapChip.h"

using namespace std;

namespace DemolisherWeapon {

	Level::Level()
	{
	}
	Level::~Level()
	{
	}

	void Level::Init(const wchar_t* levelDataFilePath, Level::HookWhenBuildObjectFunc hookFunc, const MapChipSetting& chipSetting)
	{
		//スケルトンをロードする。
		Skeleton skeleton;
		skeleton.Load(levelDataFilePath);
		//構築構築。
		//0番目はルートオブジェクトなので飛ばす。
		for (auto i = 1; i < skeleton.GetNumBones(); i++) {
			//骨を取得。
			auto bone = skeleton.GetBone(i);
			if (bone->GetParentId() == 0) {	//親がルートの場合だけマップチップを生成する。
				LevelObjectData objData;
				bone->CalcWorldTRS(objData.position, objData.rotation, objData.scale);
				//3dsMaxとは軸が違うので、補正を入れる。
				auto t = objData.position.y;
				objData.position.y = objData.position.z;
				objData.position.z = -t;

				t = objData.rotation.y;
				objData.rotation.y = objData.rotation.z;
				objData.rotation.z = -t;
				objData.name = bone->GetName();

				std::swap(objData.scale.y, objData.scale.z);

				objData.mapChipSetting = chipSetting;

				bool isHook = false;
				if (hookFunc != nullptr) {
					//hook関数が指定されているのでhook関数を呼び出す。
					isHook = hookFunc(objData);
				}
				if (isHook == false) {
					//フックされなかったので、マップチップを作成する。
					auto mapChip = std::make_unique<MapChip>(objData);
					m_mapChipArray.push_back(std::move(mapChip));
				}
			}
		}
	}

}