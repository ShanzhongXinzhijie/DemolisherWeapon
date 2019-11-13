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

		void Release() {
			//インスタンシング用リソースの開放
			m_instanceMax = 0;
			m_instanceIndex = 0; m_instanceDrawNum = 0;

			m_instancingWorldMatrix.reset();
			if (m_worldMatrixSB) { m_worldMatrixSB->Release(); m_worldMatrixSB = nullptr; }
			if (m_worldMatrixSRV) { m_worldMatrixSRV->Release(); m_worldMatrixSRV = nullptr; }
			m_instancingWorldMatrixOld.reset();
			if (m_worldMatrixSBOld) { m_worldMatrixSBOld->Release(); m_worldMatrixSBOld = nullptr; }
			if (m_worldMatrixSRVOld) { m_worldMatrixSRVOld->Release(); m_worldMatrixSRVOld = nullptr; }

			m_drawInstanceMask.reset();
			m_minAABB.reset(); m_maxAABB.reset();
			m_worldMatrixCache.reset();
			m_worldMatrixOldCache.reset();

			m_insWatchers.reset();

			m_instanceData.clear();
		}

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
			const std::shared_ptr<InstanceWatcher>& watcher
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
			m_insWatchers[m_instanceIndex] = watcher;

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
		public:
			/// <summary>
			/// カリング前に実行する処理
			/// </summary>
			/// <param name="instanceIndex">インスタンス番号</param>
			/// <returns>このインスタンスを描画するか</returns>
			//virtual bool PreCulling(int instanceIndex) {}

			/// <summary>
			/// 描画前に実行する処理
			/// 主にストラクチャーバッファの更新とSRVの設定をする
			/// </summary>
			/// <param name="instanceNum">インスタンス総数</param>
			/// <param name="drawInstanceNum">描画するインスタンスの数(カリングされなかった物の数)</param>
			/// <param name="drawInstanceMask">カリングされたインスタンスのマスク(falseになってるやつがカリングされた)</param>
			virtual void PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask) = 0;//{}

			//AddDrawInstanceで実行する処理
			//主にインスタンスごとのデータを追加する
			virtual void AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param) {}

			//SetInstanceMaxで実行する処理
			//インスタンス最大数を設定
			virtual void SetInstanceMax(int instanceMax) {}
		};

		/// <summary>
		/// IInstanceDataをセット
		/// </summary>
		/// <param name="identifier">IInstanceDataの名前</param>
		void AddIInstanceData(const wchar_t* identifier, std::unique_ptr<IInstancesData>&& IID) {
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
		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrix;
		ID3D11Buffer*				m_worldMatrixSB = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRV = nullptr;
		//旧ワールド行列
		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrixOld;
		ID3D11Buffer*				m_worldMatrixSBOld = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRVOld = nullptr;

		//視錐台カリング用
		std::unique_ptr<bool[]>		m_drawInstanceMask;
		std::unique_ptr<CVector3[]> m_minAABB, m_maxAABB;
		std::unique_ptr<CMatrix[]>	m_worldMatrixCache;
		std::unique_ptr<CMatrix[]>	m_worldMatrixOldCache;
		bool m_isFrustumCull = false;//視錐台カリングするか?

		//インスタンスたちを監視する
		std::unique_ptr<std::weak_ptr<InstanceWatcher>[]> m_insWatchers;
		
		//ユーザー設定の描画前処理
		std::function<void()> m_preDrawFunc = nullptr;

		//インスタンスごとのデータを扱う用のクラス
		std::unordered_map<int,std::unique_ptr<IInstancesData>> m_instanceData;

		//シェーダ
		Shader m_vsShader, m_vsZShader;
		Shader m_vsSkinShader, m_vsZSkinShader;
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
			const wchar_t* identifier = nullptr
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
		CInstancingModelRender();
		~CInstancingModelRender();

		//初期化
		void Init(int instanceMax,									//最大描画数
			const wchar_t* filePath,								//モデルのファイルパス
			const AnimationClip* animationClips = nullptr,			//アニメーションクリップの配列
			int numAnimationClips = 0,								//アニメーションクリップの数
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//モデルの上方向	//新規読み込み時のみ使用
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//座標系			//新規読み込み時のみ使用
			const wchar_t** identifiers = nullptr					//識別子、これが違うと別モデルとして扱われる //numAnimationClipsと同数必要
		) {
			//アニメーションの数だけモデルロード
			m_model.clear();
			for (int i = 0; i < max(numAnimationClips,1); i++) {
				const wchar_t* identifier = nullptr; if (identifiers) { identifier = identifiers[i]; }
				m_model.emplace_back(m_s_instancingModelManager.Load(instanceMax, filePath, &animationClips[i], fbxUpAxis, fbxCoordinate, identifier));
				if (m_model.back()->GetInstanceMax() < instanceMax) {
					m_model.back()->SetInstanceMax(instanceMax);
				}
			}
			m_playingAnimNum = 0;

			//バウンディングボックスの初期化
			m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);

			m_isInit = true;
		}
		
		void PostLoopUpdate()override final {
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
				m_model[m_playingAnimNum]->AddDrawInstance(m_worldMatrix, m_worldMatrixOld, m_SRTMatrix, m_scale, m_minAABB, m_maxAABB, m_ptrParam, m_watcher);
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

		//再生アニメーションの変更
		void ChangeAnim(int animNum) {
			if (animNum >= m_model.size()) { return; }
			m_playingAnimNum = animNum;
		}
		int GetPlayAnimNum()const {
			return m_playingAnimNum;
		}

		//描画するか設定
		void SetIsDraw(bool enable) {
			m_isDraw = enable;
		}
		bool GetIsDraw() const{
			return m_isDraw;
		}

		//モデルの取得
		InstancingModel* GetInstancingModel(int num) { return m_model[num]; }
		InstancingModel* GetInstancingModel() { return GetInstancingModel(m_playingAnimNum); }

		//パラメータのポインタ設定
		void SetParamPtr(void* ptrParam) {
			m_ptrParam = ptrParam;
		}		

	private:
		std::shared_ptr<InstanceWatcher> m_watcher;

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