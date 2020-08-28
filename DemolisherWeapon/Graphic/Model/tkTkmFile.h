/// <summary>
/// tkmファイル。
/// </summary>
/// <remarks>
/// tkmファイルはtkEngine独自の3Dモデルフォーマットです。
/// 本クラスはtkmファイルのローダーです。
/// </remarks>

#pragma once

namespace tkEngine {
	
	struct Noncopyable {
		Noncopyable() = default;
		Noncopyable(const Noncopyable&) = delete;
		Noncopyable& operator=(const Noncopyable&) = delete;
	};

	/// <summary>
	/// リソースのインターフェースクラス。
	/// </summary>
	class IResource : Noncopyable {
	public:
		/// <summary>
		/// デストラクタ。
		/// </summary>
		virtual ~IResource();
		/// <summary>
		/// サブクラスで実装する読み込み処理の本体。
		/// </summary>
		virtual void LoadImplement(const char* filePath) = 0;
		/// <summary>
		/// 同期ロード。
		/// </summary>
		/// <param name="filePath"></param>
		void Load(const char* filePath)
		{
			m_filePath = filePath;
			LoadImplement(filePath);
		}
		/// <summary>
		/// 非同期ロード。
		/// </summary>
		/// <param name="filePath"></param>
		void LoadAsync(const char* filePath);
		/// <summary>
		/// 読み込み終了判定。
		/// </summary>
		/// <returns></returns>
		bool IsLoaded() const
		{
			return m_isLoaded;
		}
	protected:

		/// <summary>
		/// ロード済みにする。
		/// </summary>
		void SetLoadedMark()
		{
			m_isLoaded = true;
		}
	private:
		std::string m_filePath;
		bool m_isLoaded = false;	//読み込み完了フラグ。
		std::unique_ptr< std::thread > m_loadThread;
	};

	/// <summary>
	/// tkmファイルクラス。
	/// </summary>
	class  CTkmFile : public IResource {
	public:
		/// <summary>
		/// マテリアル
		/// </summary>
		struct SMaterial {
			std::string albedoMapFileName;			//アルベドマップのファイル名。
			std::string normalMapFileName;			//法線マップのファイル名。
			std::string specularMapFileName;		//スペキュラマップのファイル名。
			std::unique_ptr<char[]>	albedoMap;		//ロードされたアルベドマップ。(ddsファイル)
			unsigned int albedoMapSize;				//アルベドマップのサイズ。(ddsファイル)
			std::unique_ptr<char[]>	normalMap;		//ロードされた法線マップ。(ddsファイル)
			unsigned int normalMapSize;				//法線マップのサイズ。
			std::unique_ptr<char[]>	specularMap;	//ロードされたスペキュラマップ。(ddsファイル)
			unsigned int specularMapSize;			//スペキュラマップのサイズ。(ddsファイル)
		};
		/// <summary>
		/// 頂点。
		/// </summary>
		struct SVertex {
			CVector3 pos;			//座標。
			CVector3 normal;		//法線。
			CVector3 tangent;		//接ベクトル。
			CVector3 binormal;		//従ベクトル。
			CVector2 uv;			//UV座標。
			int indices[4];			//スキンインデックス。
			CVector4 skinWeights;	//スキンウェイト。
		};
		/// <summary>
		/// 32ビットのインデックスバッファ。
		/// </summary>
		struct SIndexBuffer32 {
			std::vector< uint32_t > indices;	//インデックス。
		};
		/// <summary>
		/// 16ビットのインデックスバッファ。
		/// </summary>
		struct SIndexbuffer16 {
			std::vector< uint16_t > indices;	//インデックス。
		};
		/// <summary>
		/// メッシュパーツ。
		/// </summary>
		struct SMesh {
			bool isFlatShading;							//フラットシェーディング？
			std::vector< SMaterial > materials;				//マテリアルの配列。
			std::vector< SVertex >	vertexBuffer;			//頂点バッファ。
			std::vector<SIndexBuffer32> indexBuffer32Array;	//インデックスバッファの配列。マテリアルの数分だけインデックスバッファはあるよ。
			std::vector< SIndexbuffer16> indexBuffer16Array;
		};
		
		/// <summary>
		/// 3Dモデルをロード。
		/// </summary>
		/// <param name="filePath">ファイルパス。</param>
		void LoadImplement(const char* filePath) override final;
		
		/// <summary>
		/// メッシュパーツに対してクエリを行う。
		/// </summary>
		/// <param name="func">クエリ関数</param>
		void QueryMeshParts(std::function<void(const SMesh& mesh)> func) const
		{
			for (auto& mesh : m_meshParts) {
				func(mesh);
			}
		}
		/// <summary>
		/// メッシュの数を取得。
		/// </summary>
		/// <returns></returns>
		int GetNumMesh() const
		{
			return (int)(m_meshParts.size());
		}
	private:
		/// <summary>
		/// テクスチャ名をロード。
		/// </summary>
		/// <param name="fp"></param>
		/// <returns></returns>
		std::string LoadTextureFileName(FILE* fp);
		/// <summary>
		/// インデックスバッファをロード。
		/// </summary>
		template<class T>
		void LoadIndexBuffer(std::vector<T>& indexBuffer, int numIndex, FILE* fp);
		/// <summary>
		/// マテリアルを構築。
		/// </summary>
		/// <param name="tkmMat"></param>
		void BuildMaterial(SMaterial& tkmMat, FILE* fp, const char* filePath);
		/// <summary>
		/// 接ベクトルと従ベクトルを計算する。
		/// </summary>
		/// <remarks>
		/// 3dsMaxScriptでやるべきなんだろうけど、デバッグしたいので今はこちらでやる。
		/// </remarks>
		void BuildTangentAndBiNormal();
	private:
		std::vector< SMesh>	m_meshParts;		//メッシュパーツ。
	};
}
