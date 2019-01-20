/*!
 * @brief	シェーダークラス。
 */

#pragma once

namespace DemolisherWeapon {

/*!
 * @brief	シェーダー。
 * @FlyweightパターンのFlyweightクラス。
 */
class Shader {
public:
	/*!
	 * @brief	シェーダータイプ。
	 */
	enum class EnType{
		VS,			//!<頂点シェーダー。
		PS,			//!<ピクセルシェーダー。
		CS,			//!<コンピュートシェーダー。
	};
	Shader();
	~Shader();
	/*!
	 * @brief	開放。
	 * @details
	 *	この関数はデストラクタから自動的に呼ばれます。明示的なタイミングで開放を行いたい場合に使用してください。
	 */
	void Release();
	/*!
	 * @brief	ロード。
	 *@param[in]	filePath		ファイルパス。
	 *@param[in]	entryFuncName	エントリーポイントとなる関数の名前。
	 *@param[in]	shaderType		シェーダータイプ。
	 *@return	falseが返ってきたらロード失敗。
	 */
	bool Load(const char* filePath, const char* entryFuncName, EnType shaderType);
	/*!
	* @brief	
	*/
	void* GetBody()
	{
		return m_shader;
	}
	/*!
	* @brief	インプットレイアウトを取得。
	*/
	ID3D11InputLayout* GetInputLayout()
	{
		return m_inputLayout;
	}
	void* GetByteCode()
	{
		return m_blobOut->GetBufferPointer();
	}
	size_t GetByteCodeSize()
	{
		return m_blobOut->GetBufferSize();
	}

	//動的リンク関係
	//インターフェイスインスタンスの数を取得
	int GetNumInterfaces()const {
		return m_numInterfaces;
	}
	//インターフェイスインスタンスの配列を取得
	ID3D11ClassInstance** GetClassInstanceArray()const {
		return m_dynamicLinkageArray;
	}

private:
	void*				m_shader = nullptr;					//!<シェーダー。
	ID3D11InputLayout*	m_inputLayout = nullptr;			//!<入力レイアウト。
	ID3DBlob*			m_blobOut = nullptr;

	//動的リンク関係
	int m_numInterfaces = 0;
	ID3D11ClassInstance** m_dynamicLinkageArray = nullptr;
};

}