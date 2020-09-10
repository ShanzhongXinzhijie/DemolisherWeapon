#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// テクスチャデータ
	/// </summary>
	struct TextueData {
	public:
		TextueData() = default;
		TextueData(const TextueData& texd) {
			CopyFrom(texd);
		}
		TextueData& operator=(const TextueData& texd) {
			CopyFrom(texd);
			return *this;
		}
		void CopyFrom(const TextueData& mat) {
			//DirectX12
			if (mat.d3d12texture) {
				mat.d3d12texture.Get()->AddRef();
				d3d12texture.Attach(mat.d3d12texture.Get());
			}
			descriptorHandle = mat.descriptorHandle;

			//DirectX11
			if (mat.texture) {
				mat.texture.Get()->AddRef();
				texture.Attach(mat.texture.Get());
			}
			if (mat.textureView) {
				mat.textureView.Get()->AddRef();
				textureView.Attach(mat.textureView.Get());
			}

			//プロパティ
			isDDS = mat.isDDS;
			width = mat.width, height = mat.height;
		}
		bool operator==(const TextueData& rhs) const
		{
			bool judge =(d3d12texture.Get() == rhs.d3d12texture.Get())
						&& (descriptorHandle.ptr == rhs.descriptorHandle.ptr)
						&& (texture.Get() == rhs.texture.Get()) 
						&& (textureView.Get() == rhs.textureView.Get())
						&& (isDDS == rhs.isDDS)
						&& (width == rhs.width)
						&& (height == rhs.height);
			return judge;
		}
		inline bool operator!=(const TextueData& rhs) const
		{
			return !(*this == rhs);
		}

		//DirectX12
		Microsoft::WRL::ComPtr<ID3D12Resource> d3d12texture;
		D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandle = { 0 };
		//DirectX11
		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

		//プロパティ
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
	inline TextueData CreateTexture(std::experimental::filesystem::path filepath, bool generateMipmaps = false);

	/// <summary>
	/// テクスチャのFlyweightFactory
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

		bool Load(std::experimental::filesystem::path filepath, const TextueData** return_textureData = nullptr, bool generateMipmaps = false);

		/// <summary>
		/// テクスチャの開放
		/// </summary>
		void Release();

	private:		
		std::unordered_map<int, TextueData> m_textureMap;
	};
}