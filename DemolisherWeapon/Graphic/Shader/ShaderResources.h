/*!
 *@brief	シェーダーリソース。
 *@Flyweightパターンを使用。
 */

#pragma once

namespace DemolisherWeapon {

/*!
 *@brief	シェーダーリソース
 *@details
 * シングルトンパターン。
 * FlyweightパターンのFlyweightFactoryクラス。
 */
class ShaderResources{
	/*!
	*@brief	コンストラクタ。
	*/
	ShaderResources();
	/*!
	*@brief	デストラクタ。
	*/
	~ShaderResources();
public:
	
	/*!
	*@brief	シェーダーリソースのインスタンスを取得。
	*/
	static ShaderResources& GetInstance()
	{
		static ShaderResources instance;
		return instance;
	}

	//クラス リンク オブジェクトを取得
	ID3D11ClassLinkage* GetClassLinkage() const{
		return m_pClassLinkage;
	}

	/*!
	 * @brief	シェーダータイプ。
	 */
	enum class EnType {
		VS,			//!<頂点シェーダー。
		PS,			//!<ピクセルシェーダー。
		CS,			//!<コンピュートシェーダー。
	};

	/// <summary>
	/// シェーダーリソース。
	/// 読み込まれたコンパイル済みのシェーダー
	/// </summary>
	struct SShaderResource {
		void* shader = nullptr;						//!<シェーダー。
		ID3D11InputLayout* inputLayout = nullptr;	//!<入力レイアウト。
		EnType type;								//!<シェーダータイプ。
		ID3DBlob* blobOut = nullptr;

#ifndef DW_MASTER
		//マクロ
		struct D3D_SHADER_MACRO_SAVE {
			std::unique_ptr<char[]> Name;
			std::unique_ptr<char[]> Definition;
		};
		int macroNum = 0;
		std::unique_ptr<D3D_SHADER_MACRO_SAVE[]> pDefines;
		std::unique_ptr<std::string> entryFuncName;
#endif

		//動的リンク関係
		int numInterfaces = 0;
		ID3D11ClassInstance** dynamicLinkageArray = nullptr;

		//開放
		void Release(bool fullRelease = true);
	};

	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="return_resource">(戻り値)ロードしたシェーダーリソースのポインタ</param>
	/// <param name="filePath">ロードするシェーダーのファイルパス。</param>
	/// <param name="entryFuncName">エントリーポイントの関数名。</param>
	/// <param name="shaderType">シェーダータイプ。</param>
	/// <param name="definesIdentifier">識別子。マクロの組み合わせごとに違うものを使ってください</param>
	/// <param name="pDefines">マクロ定義</param>
	/// <returns>ロードが成功したか?</returns>
	bool Load(
		const ShaderResources::SShaderResource*& return_resource,
		std::string_view filePath,
		const char* entryFuncName,
		EnType shaderType,
		const char* definesIdentifier,
		const D3D_SHADER_MACRO* pDefines
	);

	/*!
	*@brief	開放。
	*@details
	* 明示的なタイミングで開放したい場合に使用してください。
	*/
	void Release();	

#ifndef DW_MASTER
	/// <summary>
	/// シェーダーのホットリロード(ゲームループから呼び出す)
	/// </summary>
	void HotReload();

	/// <summary>
	/// ファイルパスをエンジン下のものに置換するか設定
	/// </summary>
	void SetIsReplaceForEngineFilePath(bool isReplace) {
		m_replaceForEngineFilePath = isReplace;
	}
#endif

private:
	struct SShaderProgram {
		std::unique_ptr<char[]> program;
		int programSize;

#ifndef DW_MASTER
		std::string filepath;
		std::filesystem::file_time_type file_time;

		std::list<SShaderResource*> shaderResourceList;//このシェーダープログラムから作らりらたシェーダーリソースのリスト
#endif
	};

	typedef std::unique_ptr<SShaderResource> SShaderResourcePtr;
	typedef std::unique_ptr<SShaderProgram>	SShaderProgramPtr;
	std::unordered_map<int, SShaderProgramPtr>	m_shaderProgramMap;		//!<読み込み済みのシェーダープログラムのマップ。
	std::unordered_map<int, SShaderResourcePtr>	m_shaderResourceMap;	//!<シェーダーリソースのマップ。

	ID3D11ClassLinkage* m_pClassLinkage = nullptr;//クラス リンク オブジェクト

#ifndef DW_MASTER
	bool m_replaceForEngineFilePath = false;//ファイルパスをエンジン下に置き換えるか?
#endif

private:
	//シェーダプログラムをロード
	void LoadShaderProgram(const char* filePath, SShaderProgramPtr& return_prog);
	//シェーダーをコンパイルする
	bool CompileShader(const SShaderProgram* shaderProgram, const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, SShaderResource* return_resource, bool isHotReload = false);
};

}