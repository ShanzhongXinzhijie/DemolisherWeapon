/*!
 *@brief	シェーダーリソース。
 *@Flyweightパターンを使用。
 */

#pragma once

#include "Shader.h"

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
		return pClassLinkage;
	}

	/*!
	 *@brief	ロード
	 *@param[out]	shader			ロードしたシェーダーの格納先。
	 *@param[out]	inputLayout		入力レイアウトの格納先。
	 *@param[out]	blob
	 *@param[in]	filePath		ロードするシェーダーのファイルパス。
	 *@param[in]	entryFuncName	エントリーポイントの関数名。
	 *@param[in]	shaderType		シェーダータイプ。
	 */
	bool Load(
		void*& shader,
		ID3D11InputLayout*& inputLayout,
		ID3DBlob*& blob,
		const char* filePath,
		const char* entryFuncName,
		Shader::EnType shaderType,
		int& numInterfaces,
		ID3D11ClassInstance**& dynamicLinkageArray,
		const char* definesIdentifier,
		const D3D_SHADER_MACRO* pDefines
	);
	/*!
	*@brief	開放。
	*@details
	* 明示的なタイミングで開放したい場合に使用してください。
	*/
	void Release();	
private:
	struct SShaderProgram {
		std::unique_ptr<char[]> program;
		int programSize;
	};
	struct SShaderResource{
		void* shader;					//!<シェーダー。
		ID3D11InputLayout* inputLayout;	//!<入力レイアウト。
		Shader::EnType type;			//!<シェーダータイプ。
		ID3DBlob* blobOut;		

		//動的リンク関係
		int numInterfaces = 0;
		ID3D11ClassInstance** dynamicLinkageArray = nullptr;
	};
	typedef std::unique_ptr<SShaderResource>	SShaderResourcePtr;
	typedef std::unique_ptr<SShaderProgram>	SShaderProgramPtr;
	std::map<int, SShaderProgramPtr>		m_shaderProgramMap;		//!<読み込み済みのシェーダープログラムのマップ。
	std::map<int, SShaderResourcePtr>	m_shaderResourceMap;	//!<シェーダーリソースのマップ。

	ID3D11ClassLinkage* pClassLinkage = nullptr;//クラス リンク オブジェクト
};

}