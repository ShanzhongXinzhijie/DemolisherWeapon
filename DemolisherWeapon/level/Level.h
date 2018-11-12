#pragma once
#include "MapChip.h"

namespace DemolisherWeapon {

	class MapChip;

	/*!
	* @brief	オブジェクト名。
	*/
	struct LevelObjectData {
		CVector3 position;		//<座標。
		CQuaternion rotation;	//!<回転。
		CVector3 scale;			//!<拡大率。
		const wchar_t* name;	//!<名前。

		MapChipSetting mapChipSetting;//マップチップ設定

		/*!
		* @brief	引数で渡したオブジェクト名のオブジェクトか調べる。
		*@param[in]	objName		調べる名前。
		*@return	名前が同じ場合にtrueを返します。
		*/
		bool EqualObjectName(const wchar_t* objName)
		{
			return wcscmp(objName, name) == 0;
		}
		/*!
		* @brief	名前が前方一致するか調べる。
		*/
		bool ForwardMatchName(const wchar_t* n)
		{
			auto len = wcslen(n);
			auto namelen = wcslen(name);
			if (len > namelen) {
				//名前が長い。不一致。
				return false;
			}
			return wcsncmp(n, name, len) == 0;
		}
	};
	/*!
	 * @brief	レベル
	 *@
	 */
	class Level {
	public:
		Level();
		~Level();
		/*!
		* @brief	マップチップのスマートポインタの別名定義。
		*@details
		* std::unique_ptrは渡されたリソースの所有権を唯一(ユニーク)持っているように
		* 振舞うスマートポインタ。動的に確保されたリソースは、インスタンスの寿命が
		* 尽きると自動的に破棄されます。
		*/
		using MapChipPtr = std::unique_ptr<MapChip>;
		/*!
		* @brief	オブジェクトをビルドする時にフックする関数オブジェクトの型の別名定義。
		*@details
		* 引数にはオブジェクトの情報が渡されます。
		* この関数がfalseを返すと、オブジェクトの情報から、静的オブジェクトのMapChipクラスのインスタンスが
		* 生成されます。
		* オブジェクトの名前などで、ドアやジャンプ台、アイテムなどの特殊なクラスのインスタンスを生成したときに、
		* デフォルトで作成されるMapChipクラスのインスタンスが不要な場合はtrueを返してください。
		* 例えば、フック関数の中で、渡されたオブジェクトデータの名前のモデルを描画するクラスのインスタンスを
		* 生成したときに、falseを返してしまうと、同じモデルが二つ描画されることになります。

		*/
		using HookWhenBuildObjectFunc = std::function<bool(LevelObjectData& objData)>;
		/*!
		* @brief	レベルを初期化。
		*@param[in]	levelDataFilePath		tklファイルのファイルパス。
		*@param[in] hookFunc				オブジェクトを作成する時の処理をフックするための関数オブジェクト。
		*									フックしないならnullptrを指定すればよい、
		*									詳細はHookWhenBuildObjectFuncのコメントを参照。
		*@param[in]	chipSetting				マップチップの設定。
		*/
		void Init(const wchar_t* levelDataFilePath, HookWhenBuildObjectFunc hookFunc = nullptr, const MapChipSetting& chipSetting = {}, const CVector3& offsetPos = CVector3::Zero(), CQuaternion offsetRot = CQuaternion::Identity(), float offsetScale = 1.0f);
		
	private:
		std::vector<MapChipPtr> m_mapChipArray;		//!<マップチップの可変長配列。
	};

}