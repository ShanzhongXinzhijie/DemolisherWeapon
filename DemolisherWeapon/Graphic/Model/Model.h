#pragma once
#include"tktkmfile.h"

namespace DemolisherWeapon{

	/// <summary>
	/// メッシュパーツ
	/// </summary>
	class CMeshParts {
	public:
		/// <summary>
		/// メッシュ
		/// </summary>
		struct SMesh {
			std::unique_ptr<IVertexBuffer>					m_vertexBuffer;		//頂点バッファ。
			std::vector<std::unique_ptr<IIndexBuffer>>		m_indexBufferArray;	//インデックスバッファ。
			std::vector<std::unique_ptr<MaterialSetting>>	m_materials;		//マテリアル。
			std::vector<int>								m_skinFlags;		//スキンを持っているかどうかのフラグ。
		};

		/// <summary>
		/// tkmファイルから初期化。
		/// </summary>
		/// <param name="tkmFile"></param>
		void InitFromTkmFile(const tkEngine::CTkmFile& tkmFile);

		/// <summary>
		/// 描画。
		/// </summary>
		void Draw();

		std::vector< std::unique_ptr<SMesh> > m_meshs;//メッシュ

	private:
		void CreateMeshFromTkmMesh(const tkEngine::CTkmFile::SMesh& tkmMesh, int meshNo);

	};

	/// <summary>
	/// モデルクラス。
	/// </summary>
	class CModel {
	public:
		//メッシュ・マテリアルの検索

		/// <summary>
		/// tkmファイルを非同期ロ―ド。
		/// </summary>
		/// <remarks>
		/// IsInited関数を利用して、同期をとるようにしてください。
		/// </remarks>
		/// <param name="filePath">tkmファイルのファイルパス。</param>
		void LoadTkmFileAsync(const char* filePath);

		/// <summary>
		/// tkmファイルをロード。
		/// </summary>
		/// <param name="filePath">tkmファイルのファイルパス。</param>
		void LoadTkmFile(const char* filePath);

		/// <summary>
		/// メッシュパーツを作成する。
		/// </summary>
		/// <remarks>
		/// この関数はLoadTkmFileAsync関数などを利用して、
		/// tkmファイルをロードした後で呼び出してください。
		/// </remarks>
		void CreateMeshParts();

		/// <summary>
		/// 初期化が終わっているか判定。
		/// </summary>
		/// <returns></returns>
		bool IsInited() const;

		/// <summary>
		/// 描画
		/// </summary>
		void Draw();

		/// <summary>
		/// tkmファイルを取得。
		/// </summary>
		/// <returns></returns>
		const tkEngine::CTkmFile& GetTkmFile() const
		{
			return m_tkmFile;
		}

	private:
		tkEngine::CTkmFile m_tkmFile;	//tkmファイル。
		CMeshParts m_meshParts;			//メッシュパーツ。
	};
}