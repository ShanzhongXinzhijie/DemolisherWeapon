#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// インポスターのインスタンシング描画におけるテクスチャインデックスを扱うクラス
	/// </summary>
	class InstancingImposterIndex : public GameObj::InstancingModel::IInstancesData {
	private:
		void Reset(int instancingMaxNum);
	public:
		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param name="instancingMaxNum">インスタンス最大数</param>
		InstancingImposterIndex(int instancingMaxNum);
		void PreDrawUpdate()override;
		void PostLoopPostUpdate()override;
	public:
		//インスタンス最大数を設定
		void SetInstanceMax(int instanceMax);
	
		/// <summary>
		/// このフレームに描画するインスタンスの追加
		/// </summary>
		/// <param name="x">横インデックス</param>
		/// <param name="y">縦インデックス</param>
		void AddDrawInstance(int x, int y, const CVector3& pos, float scale);
		
		//インデックスの取得
		const std::unique_ptr<int[][2]>& GetIndexs()const {
			return m_instancingIndex;
		}
		//インデックスの設定と更新
		void SetUpdateDrawIndex(const int index[][2]) {
			//設定
			for (int i = 0; i < m_instanceDrawNum; i++) {
				m_instancingIndex[i][0] = index[i][0];
				m_instancingIndex[i][1] = index[i][1];
			}
			//StructuredBufferを更新
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
				m_indexSB.Get(), 0, NULL, m_instancingIndex.get(), 0, 0
			);
		}

		//座標取得
		const std::unique_ptr<CVector3[]>& GetPoses()const {
			return m_instancingPos;
		}
		//スケールの取得
		const std::unique_ptr<float[]>& GetScales()const {
			return m_instancingScale;
		}

	private:
		std::unique_ptr<int[][2]>							m_instancingIndex;
		std::unique_ptr<CVector3[]>							m_instancingPos;
		std::unique_ptr<float[]>							m_instancingScale;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_indexSB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_indexSRV;
		int m_instanceNum = 0, m_instanceDrawNum = 0;
		int m_instanceMax = 0;
	};

	/// <summary>
	/// インポスターのテクスチャを作るクラス
	/// </summary>
	class ImposterTexRender {
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="filepath">3Dモデルのファイルパス</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum);

		/// <summary>
		/// モデルのサイズを取得
		/// </summary>
		/// <returns>モデルのサイズ</returns>
		float GetModelSize()const {
			return m_imposterMaxSize;
		}

		/// <summary>
		/// モデルのカメラへの方向のサイズを得る
		/// </summary>
		/// <param name="x">横インデックス</param>
		/// <param name="y">縦インデックス</param>
		/// <returns>モデルのカメラへの方向のサイズ</returns>
		float GetDirectionOfCameraSize(int x, int y)const {
			return m_toCamDirSize[m_partNumY-1+y][x];
		}

		/// <summary>
		/// テクスチャ分割数の取得
		/// </summary>
		/// <returns>テクスチャ分割数</returns>
		UINT GetPartNumX()const { return m_partNumX; }
		UINT GetPartNumY()const { return m_partNumY; }

		/// <summary>
		/// テクスチャの種類
		/// </summary>
		enum EnGBuffer {
			enGBufferAlbedo,
			enGBufferNormal,
			enGBufferLightParam,
			enGBufferNum,
		};

		/// <summary>
		/// SRVの取得
		/// </summary>
		/// <param name="type">取得するテクスチャの種類</param>
		/// <returns>SRV</returns>
		ID3D11ShaderResourceView* GetSRV(EnGBuffer type)const {
			return m_GBufferSRV[type].Get();
		}

	private:
		//インポスタテクスチャの作成
		void Render(SkinModel& model);

	private:
		//各テクスチャ		
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_GBufferTex[enGBufferNum];	//GBufferテクスチャ
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_GBufferView[enGBufferNum];//GBufferビュー
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_GBufferSRV[enGBufferNum];	//GBufferSRV
		
		UINT m_gbufferSizeX = 0, m_gbufferSizeY = 0;//テクスチャサイズ
		UINT m_partNumX = 0, m_partNumY = 0;		//テクスチャ分割数
		float m_imposterMaxSize;//モデルの大きさ
		CVector3 m_boundingBoxMaxSize, m_boundingBoxMinSize;
		std::vector<std::vector<float>> m_toCamDirSize;//分割された各テクスチャのモデルのカメラ方向の大きさ
	};

	/// <summary>
	/// ImposterTexRenderを管理するクラス
	/// FlyweightパターンのFlyweightFactoryクラス
	/// </summary>
	class ImposterTexBank {
	//シングルトン
	private:
		ImposterTexBank() = default;
		~ImposterTexBank() { Release(); }
	public:
		ImposterTexBank(const ImposterTexBank&) = delete;
		ImposterTexBank& operator=(const ImposterTexBank&) = delete;
		ImposterTexBank(ImposterTexBank&&) = delete;
		ImposterTexBank& operator=(ImposterTexBank&&) = delete;
	public:
		//インスタンスを取得
		static ImposterTexBank& GetInstance()
		{
			if (instance == nullptr) {
				instance = new ImposterTexBank;
			}
			return *instance;
		}
	private:
		static ImposterTexBank* instance;
		//インスタンスの削除
		static void DeleteInstance() {
			if (instance) {
				delete instance; instance = nullptr;
			}
		}

	public:
		/// <summary>
		/// インポスターテクスチャのロード
		/// </summary>
		/// <param name="filepath">3Dモデルのファイルパス</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数</param>
		/// <returns>インポスターテクスチャ</returns>
		ImposterTexRender* Load(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum);

		/// <summary>
		/// テクスチャの開放
		/// </summary>
		void Release();

	private:
		std::unordered_map<int, ImposterTexRender*> m_impTexMap;
	};

namespace GameObj {

	class CImposter : public IGameObject
	{
	public:
		void PostLoopUpdate()override { ImposterUpdate(false); }
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="filepath">3Dモデルのファイルパス</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数 ※奇数を推奨?</param>
		/// <param name="instancingNum">インスタンシング描画数</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum, int instancingNum = 1);
		
		//ワールド行列とインデックスの計算
		static void CalcWorldMatrixAndIndex(bool isShadowDrawMode, const SkinModel& model, const ImposterTexRender& texture, const CVector3& pos, float scale, CVector3& position_return, CQuaternion& rotation_return, float& scale_return, int& index_x_return, int& index_y_return);

		//インポスターの更新
		void ImposterUpdate(bool isShadowDrawMode);

		//座標・拡大の設定
		void SetPos(const CVector3& pos) {
			m_pos = pos;
		}
		void SetScale(float scale) {
			m_scale = scale;
		}

		//座標・拡大の取得
		const CVector3& GetPos() const {
			return m_pos;
		}
		float GetScale() const {
			return m_scale;
		}

		//描画するか設定
		void SetIsDraw(bool flag) {
			m_billboard.SetIsDraw(flag);
		}
		
	private:
		bool m_isInit = false;
		//テクスチャ
		ImposterTexRender* m_texture = nullptr;
		//ビルボード
		CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		Shader m_zShader;
		CVector3 m_pos;
		float m_scale = 1.0f;
		//インスタンシング用
		InstancingImposterIndex* m_instancingIndex = nullptr;
	};
}
	/// <summary>
	/// インポスターのシャドウマップ描画時に実行する処理
	/// </summary>
	class ShodowWorldMatrixCalcerImposter : public ShadowMapRender::IPrePost {
	public:
		ShodowWorldMatrixCalcerImposter(GameObj::CImposter* imp, SkinModel* model);
		void PreDraw()override;
		void PreModelDraw()override;
		void PostDraw()override;
	private:
		int m_x = 0, m_y = 0;
		CMatrix	m_worldMatrix;
		GameObj::CImposter* m_ptrImposter = nullptr;
		SkinModel* m_ptrModel = nullptr;
	};
	//インスタンシング用
	class ShodowWorldMatrixCalcerInstancingImposter : public ShadowMapRender::IPrePost {
	public:
		ShodowWorldMatrixCalcerInstancingImposter(ImposterTexRender* tex, GameObj::InstancingModel* model, InstancingImposterIndex* index);
		void PreDraw()override;
		void PreModelDraw()override;
		void PostDraw()override;
	private:
		int m_instancesNum = 0;
		std::unique_ptr<int[][2]> m_index, m_indexNew;
		std::unique_ptr<CMatrix[]>	m_worldMatrix, m_worldMatrixNew;
		ImposterTexRender* m_ptrTexture = nullptr;
		GameObj::InstancingModel* m_ptrModel = nullptr;
		InstancingImposterIndex* m_ptrIndex = nullptr;
	};
}