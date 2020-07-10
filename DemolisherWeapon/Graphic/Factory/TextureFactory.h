#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// テクスチャデータ
	/// </summary>
	struct TextueData {
	public:
#ifdef DW_DX12
		Microsoft::WRL::ComPtr<ID3D12Resource> d3d12texture;
#endif
		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
//#endif
		bool isDDS = false;//ファイルソースが.ddsかどうか
		UINT width = 0, height = 0;//解像度

		//ロード済み?
		bool isLoaded()const {
			return (width > 0 && height > 0) ? true : false;
		}
	};

	/// <summary>
	/// テクスチャを作成する
	/// </summary>
	TextueData CreateTexture(std::experimental::filesystem::path filepath, bool generateMipmaps = false);

	/// <summary>
	/// テクスチャ(ビルボード)のFlyweightFactory
	/// </summary>
	class TextureFactory {
	//シングルトン
	private:
		TextureFactory() = default;
		~TextureFactory() { Release(); }
	public:
		TextureFactory(const TextureFactory&) = delete;
		TextureFactory& operator=(const TextureFactory&) = delete;
		TextureFactory(TextureFactory&&) = delete;
		TextureFactory& operator=(TextureFactory&&) = delete;

	private:
		static TextureFactory* instance;
	public:
		//インスタンスを取得
		static TextureFactory& GetInstance()
		{
			if (instance == nullptr) {
				instance = new TextureFactory;
			}
			return *instance;
		}
		//インスタンスの削除
		static void DeleteInstance() {
			if (instance) {
				delete instance; instance = nullptr;
			}
		}
	//

	public:
		/// <summary>
		/// テクスチャのロード
		/// </summary>
		/// <param name="filepath">ファイルパス</param>
		/// <param name="return_texture">テクスチャリソースが返ってくる</param>
		/// <param name="return_textureView">テクスチャSRVが返ってくる</param>
		/// <param name="return_textureData">テクスチャのデータが返ってくる</param>
		/// <param name="generateMipmaps">ミップマップを生成するか?</param>
		bool Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture = nullptr, ID3D11ShaderResourceView** return_textureView = nullptr, const TextueData** return_textureData = nullptr, bool generateMipmaps = false);

		/// <summary>
		/// テクスチャの開放
		/// </summary>
		void Release();

	private:		
		std::unordered_map<int, TextueData> m_textureMap;
	};
}