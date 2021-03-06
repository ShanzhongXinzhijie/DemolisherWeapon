#pragma once

namespace DemolisherWeapon {

	class InstanceWatcher;

namespace GameObj {

	//インスタンシング用モデル
	class InstancingModel : public IQSGameObject {
	public:
		InstancingModel() = default;
		~InstancingModel() { Release(); };
						
		void PreLoopUpdate()override;

	public:
		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param name="instanceMax">最大インスタンス数</param>
		/// <param name="filePath">モデルのファイルパス</param>
		/// <param name="animationClip">アニメーションクリップ</param>
		/// <param name="fbxUpAxis">上方向軸</param>
		/// <param name="fbxCoordinate">座標系</param>
		InstancingModel(int instanceMax,
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		) {
			Init(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
		}

		//リセット・リソースの開放
		void Release();

		//初期化
		void Init(int instanceMax,
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		);

		//最大インスタンス数の設定
		void SetInstanceMax(int instanceMax);
		int  GetInstanceMax()const { return m_instanceMax; }

		//描画インスタンス数の取得
		//int GetDrawInstanceNum()const { return m_instanceDrawNum; }

		//モデルの取得
		GameObj::CSkinModelRender& GetModelRender() { return m_model; }

		//描画前にやる処理を設定
		void SetPreDrawFunction(std::function<void()> func) {
			m_preDrawFunc = func;
		}

		//このフレームに描画するインスタンスの追加
		void AddDrawInstance(
			const CMatrix& woridMatrix, const CMatrix& woridMatrixOld,
			const CMatrix& SRTMatrix, const CVector3& scale,
			const CVector3& minAABB, const CVector3& maxAABB, 
			void *param_ptr,
			bool **rtn_isDraw
			//const std::shared_ptr<InstanceWatcher>& watcher
		) {
			if (m_instanceIndex >= m_instanceMax) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "【InstancingModel】インスタンスの最大数に達しています！\nモデル名:%ls\nインスタンス最大数:%d\n", m_model.GetSkinModel().GetModelName(), m_instanceMax);
				OutputDebugStringA(message);
#endif				
				return;
			}

			m_worldMatrixCache[m_instanceIndex] = woridMatrix;
			m_worldMatrixOldCache[m_instanceIndex] = woridMatrixOld;
			m_minAABB[m_instanceIndex] = minAABB;
			m_maxAABB[m_instanceIndex] = maxAABB;

			//IInstanceDataの処理実行
			for (auto& IID : m_instanceData) {
				IID.second->AddDrawInstance(m_instanceIndex, SRTMatrix, scale, param_ptr);
			}

			//監視者登録
			//m_insWatchers[m_instanceIndex] = watcher;
			*rtn_isDraw = &m_isDraw[m_instanceIndex];
			m_isDraw[m_instanceIndex] = true;//描画するものとして扱う

			m_instanceIndex++;
		}

		//視錐台カリングを行うか設定
		void SetIsFrustumCulling(bool enable) {
			m_isFrustumCull = enable;
		}
		bool GetIsFrustumCulling()const {
			return m_isFrustumCull;
		}

		/// <summary>
		/// インスタンスごとのデータを扱う用のインターフェイスクラス 
		/// </summary>
		class IInstancesData {
		public:
			virtual ~IInstancesData() {};

			/// <summary>
			/// このデータに対応するインスタンシングモデルを設定
			/// </summary>
			void SetInstancingModel(InstancingModel* model) {
				m_insModel = model;
			}

		public:
			/// <summary>
			/// 描画前に実行する処理
			/// 主にストラクチャーバッファの更新とSRVの設定をする
			/// </summary>
			/// <param name="instanceNum">インスタンス総数</param>
			/// <param name="drawInstanceNum">描画するインスタンスの数(カリングされなかった物の数)</param>
			/// <param name="drawInstanceMask">カリングされたインスタンスのマスク(falseになってるやつがカリングされた)</param>
			virtual void PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask) = 0;//{}

			/// <summary>
			/// AddDrawInstanceで実行する処理
			/// 主にインスタンスごとのデータを追加する
			/// </summary>
			/// <param name="instanceIndex">インスタンス番号</param>
			/// <param name="SRTMatrix">ワールド行列</param>
			/// <param name="scale">スケール</param>
			/// <param name="param">設定されたパラメータ</param>
			virtual void AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param) {}

			/// <summary>
			/// SetInstanceMaxで実行する処理
			/// インスタンス最大数を設定
			/// </summary>
			virtual void SetInstanceMax(int instanceMax) {}

			/// <summary>
			/// 描画するインスタンスのワールド行列を取得
			/// </summary>
			/// <param name="drawInstanceIndex"></param>
			/// <returns></returns>
			const CMatrix& GetDrawInstanceWorldMatrix(int drawInstanceIndex)const {
				return m_insModel->m_worldMatrixSB.GetData()[drawInstanceIndex];
			}

		private:
			InstancingModel* m_insModel = nullptr;//このデータに対応するインスタンシングモデル
		};

		/// <summary>
		/// IInstanceDataをセット
		/// </summary>
		/// <param name="identifier">IInstanceDataの名前</param>
		void AddIInstanceData(const wchar_t* identifier, std::unique_ptr<IInstancesData>&& IID) {
			IID->SetInstancingModel(this);
			int index = Util::MakeHash(identifier);
			m_instanceData.emplace(index,std::move(IID));
		}
		/// <summary>
		/// 設定されているIInstanceDataを取得
		/// </summary>
		/// <param name="identifier">IInstanceDataの名前</param>
		IInstancesData* GetIInstanceData(const wchar_t* identifier)const {
			int index = Util::MakeHash(identifier);
			auto IID = m_instanceData.find(index);
			if (IID == m_instanceData.end()) {
				return nullptr;
			}
			else {
				return IID->second.get();
			}
		}
		/// <summary>
		/// 設定されているIInstanceDataを削除
		/// </summary>
		/// <param name="identifier">IInstanceDataの名前</param>
		void DeleteIInstanceData(const wchar_t* identifier) {
			int index = Util::MakeHash(identifier);
			auto IID = m_instanceData.find(index);
			if (IID == m_instanceData.end()) {
				//要素が見つからない
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "【InstancingModel::DeleteIInstanceData】要素が見つかりません。要素名:%ls \n", identifier);
				DW_WARNING_MESSAGE(true, message);
#endif
				return;
			}
			else {
				m_instanceData.erase(index);
			}
		}

	private:
		int m_instanceIndex = 0, m_instanceDrawNum = 0;
		int m_instanceMax = 0;

		//モデル
		GameObj::CSkinModelRender m_model;
		AnimationClip m_animationClip;

		//ワールド行列
		StructuredBuffer<CMatrix>	m_worldMatrixSB;

		//旧ワールド行列
		StructuredBuffer<CMatrix>	m_worldMatrixSBOld;

		//視錐台カリング用
		std::unique_ptr<bool[]>		m_isDraw;
		std::unique_ptr<bool[]>		m_drawInstanceMask;
		std::unique_ptr<CVector3[]> m_minAABB, m_maxAABB;
		std::unique_ptr<CMatrix[]>	m_worldMatrixCache;
		std::unique_ptr<CMatrix[]>	m_worldMatrixOldCache;
		bool m_isFrustumCull = false;//視錐台カリングするか?

		//インスタンスたちを監視する
		//std::unique_ptr<std::weak_ptr<InstanceWatcher>[]> m_insWatchers;
		
		//ユーザー設定の描画前処理
		std::function<void()> m_preDrawFunc = nullptr;

		//インスタンスごとのデータを扱う用のクラス
		std::unordered_map<int,std::unique_ptr<IInstancesData>> m_instanceData;

		//シェーダ
		static inline bool m_s_isShaderLoaded = false;
		static inline Shader m_s_vsShader, m_s_vsZShader;
		static inline Shader m_s_vsSkinShader, m_s_vsZSkinShader;
	};

}
	//インスタンシング用モデルのマネージャー
	class InstancingModelManager {
	public:
		~InstancingModelManager() {
			Release();
		}

		void Release() {
			//マップ内のすべてのInstancingModelを解放
			for(auto& p : m_instancingModelMap){
				delete p.second;
			}
		}

		//指定のモデルを削除
		void Delete(const wchar_t* filePath, const AnimationClip* animationClip = nullptr, const wchar_t* identifier = nullptr);

		//モデルのロード
		GameObj::InstancingModel* Load(
			int instanceMax,										//新規読み込み時のみ使用
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//新規読み込み時のみ使用
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//新規読み込み時のみ使用
			const wchar_t* identifier = nullptr,
			bool* return_isNewload = nullptr
		);

	private:
		typedef std::tuple<std::size_t, std::size_t, std::size_t> key_t;

		struct key_hash 
		{
			std::size_t operator()(const key_t& k) const
			{
				return Util::HashCombine(Util::HashCombine(std::get<0>(k), std::get<1>(k)), std::get<2>(k));
			}
		};

		std::unordered_map<key_t, GameObj::InstancingModel*, key_hash> m_instancingModelMap;
	};

	

namespace GameObj {

	//インスタンシング用モデルのレンダラー
	class CInstancingModelRender : public IQSGameObject
	{
	public:
		CInstancingModelRender(bool isRegister = true);
		~CInstancingModelRender();

		//初期化
		//戻り値: 新規モデルロードが発生したか
		bool Init(int instanceMax,									//最大描画数
			const wchar_t* filePath,								//モデルのファイルパス
			const AnimationClip* animationClips = nullptr,			//アニメーションクリップの配列
			int numAnimationClips = 0,								//アニメーションクリップの数
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//モデルの上方向	//新規読み込み時のみ使用
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//座標系			//新規読み込み時のみ使用
			std::wstring_view identifiers[] = nullptr				//識別子、これが違うと別モデルとして扱われる //numAnimationClipsと同数必要
		) {
			bool returnIsNew = false;

			//アニメーションの数だけモデルロード
			m_model.clear();
			for (int i = 0; i < max(numAnimationClips,1); i++) {
				const wchar_t* identifier = nullptr; if (identifiers) { identifier = identifiers[i].data(); }
				bool isNew = false;
				m_model.emplace_back(m_s_instancingModelManager.Load(instanceMax, filePath, &animationClips[i], fbxUpAxis, fbxCoordinate, identifier, &isNew));
				if (isNew) {
					returnIsNew = true;
				}
				if (m_model.back()->GetInstanceMax() < instanceMax) {
					m_model.back()->SetInstanceMax(instanceMax);
				}
			}
			m_playingAnimNum = 0;

			//バウンディングボックスの初期化
			m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);

			m_isInit = true;

			return returnIsNew;
		}
		bool Init(int instanceMax,									//最大描画数
			const wchar_t* filePath,								//モデルのファイルパス
			std::wstring_view identifier							//識別子、これが違うと別モデルとして扱われる
		) {
			return Init(instanceMax, filePath, nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, &identifier);
		}

		void PostLoopPostUpdate()override final {
			if (!m_isInit) { return; }
			if (!m_isDraw) { m_isFirstWorldMatRef = true; return; }

			//ワールド行列、更新してなければ
			if (!m_isUpdatedWorldMatrix) {
				if (m_isSetRotOrScale) {
					//ワールド行列を求める(バイアス含む)
					m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcWorldMatrix(m_pos, m_rot, m_scale, m_worldMatrix, m_SRTMatrix);
					m_isSetRotOrScale = false;
				}
				else {
					//平行移動部分を更新
					m_worldMatrix.SetTranslation(m_pos);
					m_SRTMatrix.SetTranslation(m_pos);
				}
				//バウンディングボックスの更新
				m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);
			}

			//最初のワールド座標更新なら...
			if (m_isFirstWorldMatRef) {
				m_isFirstWorldMatRef = false;
				//旧座標の更新
				m_worldMatrixOld = m_worldMatrix;
			}

			//インスタンシングモデルに送る
			if (m_isDraw) {
				m_model[m_playingAnimNum]->AddDrawInstance(m_worldMatrix, m_worldMatrixOld, m_SRTMatrix, m_scale, m_minAABB, m_maxAABB, m_ptrParam, &m_insIsDrawPtr);// m_watcher);
			}

			//更新してなければ
			if (!m_isUpdatedWorldMatrix) {
				m_worldMatrixOld = m_worldMatrix;
				m_isUpdatedWorldMatrix = true;//更新済みにする
			}
		}

		//座標とか設定
		void SetPos(const CVector3& pos) {
			m_isUpdatedWorldMatrix = false;
			m_pos = pos;
		}
		void SetRot(const CQuaternion& rot) {
			m_isUpdatedWorldMatrix = false; m_isSetRotOrScale = true;
			m_rot = rot;
		}
		void SetScale(const CVector3& scale) {
			m_isUpdatedWorldMatrix = false; m_isSetRotOrScale = true;
			m_scale = scale;
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//座標とか取得
		const CVector3& GetPos()const {
			return m_pos;
		}
		const CQuaternion& GetRot()const {
			return m_rot;
		}
		const CVector3& GetScale()const {
			return m_scale;
		}
		const CMatrix& GetWorldMatrix()const {
			return m_worldMatrix;
		}

		//再生アニメーションの変更
		void ChangeAnim(int animNum) {
			if (animNum >= m_model.size()) { return; }
			m_playingAnimNum = animNum;
		}
		int GetPlayAnimNum()const {
			return m_playingAnimNum;
		}

		//描画するか設定
		//※実行タイミングによってはダングリングポインタへのアクセスが発生します(面倒なので放置)
		void SetIsDraw(bool enable) {
			m_isDraw = enable;
			if (m_insIsDrawPtr) { *m_insIsDrawPtr = enable; }
		}
		bool GetIsDraw() const{
			return m_isDraw;
		}
		//旧ワールド行列のリセット
		void ResetWorldMatrixOld() {
			m_isFirstWorldMatRef = true;
		}

		//モデルの取得
		InstancingModel* GetInstancingModel(int num) { return m_model[num]; }
		InstancingModel* GetInstancingModel() { return GetInstancingModel(m_playingAnimNum); }

		//パラメータのポインタ設定
		void SetParamPtr(void* ptrParam) {
			m_ptrParam = ptrParam;
		}		

	private:
		bool* m_insIsDrawPtr = nullptr;//std::shared_ptr<InstanceWatcher> m_watcher;

		bool m_isInit = false;
		bool m_isDraw = true;
		
		std::vector<GameObj::InstancingModel*> m_model;
		int m_playingAnimNum = 0;

		bool m_isUpdatedWorldMatrix = false;//ワールド行列更新済みか?
		bool m_isSetRotOrScale = true;		//回転または拡大を設定したか?
		bool m_isFirstWorldMatRef = true;	//最初のワールド行列更新か?
		CVector3 m_pos;
		CQuaternion m_rot;
		CVector3 m_scale = CVector3::One();
		CMatrix m_worldMatrix, m_worldMatrixOld;
		CMatrix m_SRTMatrix;
		CVector3 m_minAABB, m_maxAABB;

		void* m_ptrParam = nullptr;

	public:
		static InstancingModelManager& GetInstancingModelManager() { return m_s_instancingModelManager; }
	private:
		static InstancingModelManager m_s_instancingModelManager;
	};

}

	//CInstancingModelRenderを監視するクラス
	class InstanceWatcher {
	public:
		void Watch(const GameObj::CInstancingModelRender* ptr) {
			m_render = ptr;
		}
		bool GetIsDraw() const {
			if (!m_render) { return false; }
			return m_render->GetIsDraw();
		}
	private:
		const GameObj::CInstancingModelRender* m_render = nullptr;
	};

}