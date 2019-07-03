#pragma once

namespace DemolisherWeapon {
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

	public:
		/// <summary>
		/// テクスチャのロード
		/// </summary>
		/// <param name="filepath">ファイルパス</param>
		/// <param name="return_texture">テクスチャリソースが返ってくる</param>
		/// <param name="return_textureView">テクスチャSRVが返ってくる</param>
		void Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture = nullptr, ID3D11ShaderResourceView** return_textureView = nullptr);

		/// <summary>
		/// テクスチャの開放
		/// </summary>
		void Release();

	private:
		struct TextueData {
			ID3D11Resource* texture = nullptr;
			ID3D11ShaderResourceView* textureView = nullptr;
		};
		std::unordered_map<int, TextueData> m_textureMap;
	};
}