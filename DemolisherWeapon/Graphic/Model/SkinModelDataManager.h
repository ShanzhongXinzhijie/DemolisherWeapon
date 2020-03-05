#pragma once

namespace DemolisherWeapon {

class Skeleton;

/*!
*@brief	スキンモデルデータマネージャー。。
*@FlyweightパターンのFlyweightFactoryクラス、。
*/
class SkinModelDataManager
{
public:
	/// <summary>
	/// モデルを作成
	/// (Mapにモデルが登録されません)
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <param name="skeleton">スケルトン</param>
	/// <returns>作成したモデルデータ</returns>
	std::unique_ptr<DirectX::Model> CreateModel(const wchar_t* filePath, const Skeleton& skeleton);
	
	/// <summary>
	/// モデルをロード
	/// (Mapにモデルが登録されます。すでに登録されている場合そこから参照を取る)
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <param name="sketon">スケルトン</param>
	/// <returns>ロードしたモデルの参照</returns>
	DirectX::Model* Load(const wchar_t* filePath, const Skeleton& sketon);

	/*!
	*@brief	モデルデータを全開放。
	*/
	void Release();

private:
	//ファイルパスをキー、DirectXModelのインスタンスを値とするマップ。
	//辞書みたいなものです。
	std::unordered_map<
		std::wstring,					//wstringがキー。 
		std::unique_ptr<DirectX::Model>	//これが値。
	> m_directXModelMap;
};

}

