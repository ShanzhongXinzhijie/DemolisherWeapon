#pragma once
#include <map>

namespace DemolisherWeapon {

class Skeleton;
/*!
*@brief	スキンモデルデータマネージャー。。
*@FlyweightパターンのFlyweightFactoryクラス、。
*/
class SkinModelDataManager
{
public:
	/*!
	*@brief	モデルをロード。
	*@param[in]	filePath	ファイルパス。
	*@param[in]	skeleton	スケルトン。
	*/
	DirectX::Model* Load(const wchar_t* filePath, const Skeleton& sketon);
	/*!
	*@brief	モデルデータを全開放。
	*/
	void Release();
private:
	//ファイルパスをキー、DirectXModelのインスタンスを値とするマップ。
	//辞書みたいなものです。
	std::map< 
		std::wstring,					//wstringがキー。 
		std::unique_ptr<DirectX::Model>	//これが値。
	> m_directXModelMap;
};

}

