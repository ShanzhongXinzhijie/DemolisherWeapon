#pragma once

namespace DemolisherWeapon {

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
		//std::vector<std::vector<CVector3>> m_fronts, m_ups;//分割された各テクスチャの前方向と上方向
		float m_imposterMaxSize;//モデルの大きさ
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
		void PostLoopUpdate()override;

	public:
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="filepath">3Dモデルのファイルパス</param>
		/// <param name="resolution">インポスターテクスチャの解像度</param>
		/// <param name="partNum">インポスターテクスチャの分割数 ※奇数を推奨?</param>
		/// <param name="instancingNum">インスタンシング描画数</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum, int instancingNum = 1);
		
		//座標・回転・拡大の設定
		void SetPos(const CVector3& pos) {
			m_pos = pos;
		}
		/*void SetRot(const CQuaternion& rot) {
			m_billboard.SetRot(rot);
		}*/
		void SetScale(float scale) {
			m_billboard.SetScale(scale*(m_texture->GetModelSize()*2.0f));
			m_scale = scale;
		}
		/*void SetPRS(const CVector3& pos, const CQuaternion& rot, float scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}*/
		//座標・回転・拡大の取得
		const CVector3& GetPos() const {
			return m_pos;
		}
		/*const CQuaternion& GetRot() const {
			return m_billboard.GetRot();
		}*/
		float GetScale() const {
			return m_scale;
		}
		/*void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const {
			*pos = GetPos();
			*rot = GetRot();
			*scale = GetScale();
		}*/

		//描画するか設定
		void SetIsDraw(bool flag) {
			m_billboard.SetIsDraw(flag);
		}
		
	private:
		bool m_isInit = false;
		//テクスチャ
		ImposterTexRender* m_texture = nullptr;
		//ラスタライザーステート
		//デプスバイアス用...
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_depthRSCw;
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_depthRSCCw;
		//ビルボード
		CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		CVector3 m_pos;
		float m_scale = 1.0f;
	};
}
}