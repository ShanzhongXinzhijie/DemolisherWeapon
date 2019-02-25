#pragma once

namespace DemolisherWeapon {

namespace GameObj {

	//インスタンシング用モデル
	class InstancingModel : public IQSGameObject {
	public:
		InstancingModel() {};
		InstancingModel(int instanceMax,
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		) {
			Init(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
		}
		~InstancingModel() {
			Release();
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

		//モデルの取得
		GameObj::CSkinModelRender& GetModelRender() { return m_model; }

		//描画前にやる処理を設定
		void SetPreDrawFunction(std::function<void()> func) {
			m_preDrawFunc = func;
		}

		//このフレームに描画するインスタンスの追加
		void AddDrawInstance(const CMatrix* woridMatrix, const CMatrix* woridMatrixOld = nullptr) {
			if (m_instanceNum + 1 >= m_instanceMax) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "【InstancingModel】インスタンスの最大数に達しています！\nモデル名:%ls\nインスタンス最大数:%d\n", m_model.GetSkinModel().GetModelName(), m_instanceMax);
				OutputDebugStringA(message);
#endif				
				return;
			}

			if (!woridMatrixOld) { woridMatrixOld = woridMatrix; }
			m_instancingWorldMatrix[m_instanceNum] = *woridMatrix;
			m_instancingWorldMatrixOld[m_instanceNum] = *woridMatrixOld;
			m_instanceNum++;
		}

		void PostLoopPostUpdate()override;

	private:
		int m_instanceNum = 0;
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
		void Init(int instanceMax,									
			const wchar_t* filePath,
			const AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//新規読み込み時のみ使用
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//新規読み込み時のみ使用
			const wchar_t** identifiers = nullptr					//numAnimationClipsと同数必要
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
		}
		
		void PostLoopUpdate()override final {
			//ワールド行列を求める(バイアス含む)
			m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcWorldMatrix( m_pos, m_rot, m_scale, m_worldMatrix);

			//最初のワールド座標更新なら...
			if (m_isFirstWorldMatRef) {
				m_isFirstWorldMatRef = false;
				//旧座標の更新
				m_worldMatrixOld = m_worldMatrix;
			}

			//インスタンシングモデルに送る
			if (m_isDraw) {
				m_model[m_playingAnimNum]->AddDrawInstance(&m_worldMatrix, &m_worldMatrixOld);
			}
			m_worldMatrixOld = m_worldMatrix;
		}

		//座標とか設定
		void SetPos(const CVector3& pos) {
			m_pos = pos;
		}
		void SetRot(const CQuaternion& rot) {
			m_rot = rot;
		}
		void SetScale(const CVector3& scale) {
			m_scale = scale;
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
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
		bool m_isDraw = true;

		std::vector<GameObj::InstancingModel*> m_model;
		int m_playingAnimNum = 0;

		bool m_isFirstWorldMatRef = true;
		CVector3 m_pos;
		CQuaternion m_rot;
		CVector3 m_scale = CVector3::One();
		CMatrix m_worldMatrix, m_worldMatrixOld;

	public:
		static InstancingModelManager& GetInstancingModelManager() { return m_s_instancingModelManager; }
	private:
		static InstancingModelManager m_s_instancingModelManager;
	};

}
}