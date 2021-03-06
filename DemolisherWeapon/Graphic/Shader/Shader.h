#pragma once

#include <dxcapi.h>

#include"ShaderResources.h"

namespace DemolisherWeapon {

/*!
 * @brief	シェーダー。
 * @FlyweightパターンのFlyweightクラス。
 */
class Shader {
public:
	using EnType = ShaderResources::EnType;

	Shader() = default;

	/// <summary>
	/// ロード。
	/// </summary>
	/// <param name="filePath">ファイルパス。</param>
	/// <param name="entryFuncName">エントリーポイントとなる関数の名前。</param>
	/// <param name="shaderType">シェーダータイプ。</param>
	/// <param name="definesIdentifier">識別子。マクロ定義ごとに違うものを使ってください</param>
	/// <param name="pDefines">マクロ定義</param>
	/// <returns>falseが返ってきたらロード失敗。</returns>
	bool Load(const char* filePath, const char* entryFuncName, EnType shaderType, const char* definesIdentifier = "", const D3D_SHADER_MACRO* pDefines = nullptr);

	/// <summary>
	/// レイトレーシング用シェーダのロード
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <returns>成功?</returns>
	bool LoadRaytracing(const wchar_t* filePath);

	/// <summary>
	/// DxcBlobの取得
	/// </summary>
	/// <returns></returns>
	IDxcBlob* GetCompiledDxcBlob()const {
		return m_dxcBlob;
	}

	/*!
	* @brief	シェーダーを取得
	*/
	void* GetBody()const
	{
		return m_pShaderResource->shader;
	}
	/*!
	* @brief	インプットレイアウトを取得。
	*/
	ID3D11InputLayout* GetInputLayout()const
	{
		return m_pShaderResource->inputLayout;
	}

	ID3DBlob* GetBlob()const {
		return m_pShaderResource->GetBlob();
	}
	void* GetByteCode()const
	{
		return m_pShaderResource->GetBufferPointer();
	}
	size_t GetByteCodeSize()const
	{
		return m_pShaderResource->GetBufferSize();
	}

	//動的リンク関係
	//インターフェイスインスタンスの数を取得
	int GetNumInterfaces()const {
		return m_pShaderResource->numInterfaces;
	}
	//インターフェイスインスタンスの配列を取得
	ID3D11ClassInstance** GetClassInstanceArray()const {
		return m_pShaderResource->dynamicLinkageArray;
	}

	//ロード済みか取得
	bool GetIsLoaded()const { return m_isLoaded; }

private:
	bool m_isLoaded = false;//ロード済み?
	const ShaderResources::SShaderResource* m_pShaderResource = nullptr;

	IDxcBlob* m_dxcBlob = nullptr;	//DXCコンパイラを使用したときのシェーダーデータ。
};

}