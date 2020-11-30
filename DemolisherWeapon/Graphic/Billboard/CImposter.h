#pragma once

namespace DemolisherWeapon {

	class ImposterTexRender;

	/// <summary>
	/// インポスターのインスタンシング描画におけるパラメータを扱うクラス
	/// </summary>
	class InstancingImposterParamManager : public GameObj::InstancingModel::IInstancesData {
	private:
		//再確保
		void Reset(int instancingMaxNum);
		//角度を追加
		void AddRotY(int instanceIndex, float rad);

	public:
		void PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask)override;
		void AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param)override;
		void SetInstanceMax(int instanceMax)override;

	public:
		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param name="instancingMaxNum">インスタンス最大数</param>
		/// <param name="tex">インポスターテクスチャ</param>
		InstancingImposterParamManager(int instancingMaxNum, ImposterTexRender* tex);		

	private:
		ImposterTexRender* m_texture = nullptr;

		//パラメータ
		//x = モデルサイズ(カメラ方向へのオフセットにかかる), y = y軸回転角度
		StructuredBuffer<CVector2>  m_paramsSB;
		std::unique_ptr<CVector2[]> m_paramsCache;

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
		/// <param name="model">3Dモデル</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数</param>
		/// <param name="rotOffset">回転オフセット</param>
		/// <param name="isJustFit">テクスチャにピッタリモデルが収まるように描画するか?(一枚絵用)</param>
		void Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum, const CQuaternion& rotOffset, bool isJustFit);

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
		float GetDirectionOfCameraSize(int x, int y)const;

		/// <summary>
		/// モデルのカメラへの方向のサイズSRVを頂点シェーダに設定
		/// </summary>
		void VSSetSizeToCameraSRV();

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
			enGBufferTranslucent,
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

		/// <summary>
		/// アルベドテクスチャデータ取得
		/// </summary>
		/// <returns></returns>
		const TextueData& GetAlbedoTextureData()const {
			return m_albedoTextureData;
		}

	private:
		/// <summary>
		/// インポスタテクスチャの作成
		/// </summary>
		/// <param name="model">モデル</param>
		/// <param name="rotOffset">回転オフセット</param>
		/// <param name="isJustFit">テクスチャにピッタリモデルが収まるように描画するか?(一枚絵用)</param>
		void Render(SkinModel& model, const CQuaternion& rotOffset, bool isJustFit, const CVector3& justFitCenter, const CVector3& justFitExpand);

	private:
		//各テクスチャ		
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_GBufferTex[enGBufferNum];	//GBufferテクスチャ
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_GBufferView[enGBufferNum];//GBufferビュー
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_GBufferSRV[enGBufferNum];	//GBufferSRV
		TextueData m_albedoTextureData;
		
		UINT m_gbufferSizeX = 0, m_gbufferSizeY = 0;//テクスチャサイズ
		UINT m_partNumX = 0, m_partNumY = 0;		//テクスチャ分割数
		float m_imposterMaxSize;//モデルの大きさ
		float m_imposterSizeZ;
		CVector3 m_boundingBoxMaxSize, m_boundingBoxMinSize;

		//分割された各テクスチャのモデルのカメラ方向の大きさ
		StructuredBuffer<float> m_toCamDirSize;
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
		/// <param name="identifier">識別名</param>
		/// <param name="model">3dモデル</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数 ※奇数を推奨?</param>
		/// <param name="rotOffset">回転オフセット</param>
		/// <param name="isJustFit">テクスチャにピッタリモデルが収まるように描画するか?(一枚絵用)</param>
		/// <returns>インポスターテクスチャ</returns>
		ImposterTexRender* Load(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, const CQuaternion& rotOffset = CQuaternion::Identity(), bool isJustFit = false);
		
		/// <summary>
		/// 一枚絵としてロード
		/// </summary>
		/// <param name="identifier">識別名</param>
		/// <param name="model">3dモデル</param>
		/// <param name="resolution">テクスチャの解像度</param>
		/// <param name="rotOffset">回転オフセット</param>
		const TextueData& LoadSprite(const wchar_t* identifier, SkinModel& model, UINT resolusuon, const CQuaternion& rotOffset = CQuaternion::Identity())
		{
			return Load(identifier, model, { (float)resolusuon,(float)resolusuon }, {1.0f,1.0f}, rotOffset, true)->GetAlbedoTextureData();
		}

		/// <summary>
		/// ロード済みのテクスチャを取得
		/// </summary>
		/// <param name="identifier">識別名</param>
		ImposterTexRender* Get(const wchar_t* identifier);

		/// <summary>
		/// テクスチャの開放
		/// </summary>
		void Release();

	private:
		std::unordered_map<int, ImposterTexRender*> m_impTexMap;
	};
	
	class CImposter
	{
	public:
		CImposter(bool isRegister = true) : m_billboard(isRegister){}

		/// <summary>
		/// 初期化(リソース新規作成)
		/// </summary>
		/// <param name="identifier">設定する識別名</param>
		/// <param name="model">テクスチャにする3Dモデル</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数 ※奇数を推奨?</param>
		/// <param name="instancingNum">インスタンシング描画数</param>
		bool Init(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, int instancingNum);
		/// <summary>
		/// 初期化(読み込み済みのリソースを使用)
		/// </summary>
		/// <param name="identifier">リソースの識別名</param>
		/// <param name="instancingNum">インスタンシング描画数</param>
		bool Init(const wchar_t* identifier, int instancingNum);

		//座標・拡大の設定
		void SetPos(const CVector3& pos) {
			m_pos = pos;
			m_billboard.SetPos(m_pos);
		}
		void SetScale(float scale) {
			m_scale = scale;
			m_billboard.SetScale(m_scale * m_texture->GetModelSize()*2.0f);
			m_billboard.GetModel().GetSkinModel().SetImposterParameter(m_scale, m_rotYrad);
		}
		//Y軸回転角度
		void SetRotY(float rad) {
			m_rotYrad = rad; 
			if (m_rotYrad < -CMath::PI2) { m_rotYrad += CMath::PI2; }
			if (m_rotYrad > CMath::PI2) { m_rotYrad -= CMath::PI2; }
			m_billboard.GetModel().GetSkinModel().SetImposterParameter(m_scale, m_rotYrad);			
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
		//シャドウマップに描画するか設定
		void SetIsShadowCaster(bool flag) {
			m_billboard.GetModel().SetIsShadowCaster(flag);
		}

		/// <summary>
		/// SRVの取得
		/// </summary>
		/// <param name="type">取得するテクスチャの種類</param>
		/// <returns>SRV</returns>
		ID3D11ShaderResourceView* GetSRV(ImposterTexRender::EnGBuffer type)const {
			return m_texture->GetSRV(type);
		}

		/// <summary>
		/// インスタンシングモデルの取得
		/// </summary>
		GameObj::CInstancingModelRender& GetInstancingModel() {
			return m_billboard.GetInstancingModel();
		}

	private:
		/// <summary>
		/// 初期化で使用する関数
		/// </summary>
		void InnerInit(const wchar_t* identifier, int instancingNum);

	private:
		bool m_isInit = false;
		//テクスチャ
		ImposterTexRender* m_texture = nullptr;
		//ビルボード
		CBillboard m_billboard;
		//シェーダ
		static inline bool m_s_isShaderLoaded = false;
		enum EnShaderType {
			enNormal, enInstancing, enShaderTypeNum
		};
		static inline SkinModelEffectShader m_s_imposterPS[enShaderTypeNum];
		static inline Shader m_s_zShader[enShaderTypeNum], m_s_vsShader[enShaderTypeNum], m_s_vsZShader[enShaderTypeNum];

		CVector3 m_pos;
		float m_scale = 1.0f, m_rotYrad = 0.0f;
	};
	
}