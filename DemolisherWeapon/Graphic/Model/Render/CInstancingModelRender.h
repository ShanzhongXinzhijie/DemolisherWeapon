#pragma once

namespace DemolisherWeapon {

namespace GameObj {

	//インスタンシング用モデル
	class InstancingModel : public IQSGameObject {
	public:
		InstancingModel() = default;
		~InstancingModel() { Release(); };

		void PostLoopPostUpdate()override;

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
			m_instanceNum = 0;

			m_instancingWorldMatrix.reset();
			if (m_worldMatrixSB) { m_worldMatrixSB->Release(); m_worldMatrixSB = nullptr; }
			if (m_worldMatrixSRV) { m_worldMatrixSRV->Release(); m_worldMatrixSRV = nullptr; }
			m_instancingWorldMatrixOld.reset();
			if (m_worldMatrixSBOld) { m_worldMatrixSBOld->Release(); m_worldMatrixSBOld = nullptr; }
			if (m_worldMatrixSRVOld) { m_worldMatrixSRVOld->Release(); m_worldMatrixSRVOld = nullptr; }
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
		int GetDrawInstanceNum()const { return m_instanceDrawNum; }

		//モデルの取得
		GameObj::CSkinModelRender& GetModelRender() { return m_model; }

		//描画前にやる処理を設定
		void SetPreDrawFunction(std::function<void()> func) {
			m_preDrawFunc = func;
		}

		//このフレームに描画するインスタンスの追加
		void AddDrawInstance(const CMatrix& woridMatrix, const CMatrix& woridMatrixOld, const CMatrix& SRTMatrix, const CVector3& scale) {
			if (m_instanceNum + 1 >= m_instanceMax) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "【InstancingModel】インスタンスの最大数に達しています！\nモデル名:%ls\nインスタンス最大数:%d\n", m_model.GetSkinModel().GetModelName(), m_instanceMax);
				OutputDebugStringA(message);
#endif				
				return;
			}

			m_instancingWorldMatrix[m_instanceNum] = woridMatrix;
			m_instancingWorldMatrixOld[m_instanceNum] = woridMatrixOld;

			//IInstanceDataの処理実行
			if (m_instanceData) { m_instanceData->AddDrawInstance(m_instanceNum, SRTMatrix, scale); }

			m_instanceNum++;
		}

		/// <summary>
		/// ワールド行列の取得
		/// </summary>
		const auto& GetWorldMatrix()const {
			return m_instancingWorldMatrix;
		}
		/// <summary>
		/// ワールド行列の設定と更新
		/// </summary>
		void SetUpdateDrawWorldMatrix(const CMatrix* mat) {
			//設定
			for (int i = 0; i < GetDrawInstanceNum(); i++) {
				m_instancingWorldMatrix[i] = mat[i];
			}
			//StructuredBufferを更新。
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
				m_worldMatrixSB, 0, NULL, m_instancingWorldMatrix.get(), 0, 0
			);
		}

		//インスタンスごとのデータを扱う用のインターフェイスクラス 
		class IInstancesData {
		public:
			virtual ~IInstancesData() {};
		public:
			//描画前に実行する処理
			//主にSRVの設定をする
			virtual void PreDrawUpdate() = 0;

			//PostLoopPostUpdateで実行する処理
			//主にストラクチャーバッファの更新をする
			virtual void PostLoopPostUpdate() = 0;

			//AddDrawInstanceで実行する処理
			//主にインスタンスごとのデータを追加する
			virtual void AddDrawInstance(int instanceNum, const CMatrix& SRTMatrix, const CVector3& scale) {}

			//SetInstanceMaxで実行する処理
			//インスタンス最大数を設定
			virtual void SetInstanceMax(int instanceMax) {}
		};
		/// <summary>
		/// IInstanceDataをセット
		/// </summary>
		void SetIInstanceData(std::unique_ptr<IInstancesData>&& IID) {
			m_instanceData = std::move(IID);
		}
		/// <summary>
		/// 設定されているIInstanceDataを取得
		/// </summary>
		IInstancesData* GetIInstanceData()const {
			return m_instanceData.get();
		}

	private:
		int m_instanceNum = 0, m_instanceDrawNum = 0;
		int m_instanceMax = 0;

		GameObj::CSkinModelRender m_model;
		AnimationClip m_animationClip;

		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrix;
		ID3D11Buffer*				m_worldMatrixSB = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRV = nullptr;

		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrixOld;
		ID3D11Buffer*				m_worldMatrixSBOld = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRVOld = nullptr;

		std::function<void()> m_preDrawFunc = nullptr;

		std::unique_ptr<IInstancesData> m_instanceData;

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
			}

			//最初のワールド座標更新なら...
			if (m_isFirstWorldMatRef) {
				m_isFirstWorldMatRef = false;
				//旧座標の更新
				m_worldMatrixOld = m_worldMatrix;
			}

			//インスタンシングモデルに送る
			if (m_isDraw) {
				m_model[m_playingAnimNum]->AddDrawInstance(m_worldMatrix, m_worldMatrixOld, m_SRTMatrix, m_scale);
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

	private:
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

	public:
		static InstancingModelManager& GetInstancingModelManager() { return m_s_instancingModelManager; }
	private:
		static InstancingModelManager m_s_instancingModelManager;
	};

}
}