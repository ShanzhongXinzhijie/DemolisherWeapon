#include "DWstdafx.h"
#include "CInstancingModelRender.h"
#include "Graphic/Model/SkinModelShaderConst.h"
#include "Graphic/FrustumCulling.h"

namespace DemolisherWeapon {

	namespace {
		//キーを作成
		std::tuple<std::size_t, std::size_t, std::size_t> CreateInstancingModelMapKey(const wchar_t* filePath, const AnimationClip* animationClip, const wchar_t* identifier) {
			std::size_t modelHash, animHash, idenHash;
			modelHash = Util::MakeHash(filePath);
			if (animationClip) { animHash = animationClip->GetHash(); } else { animHash = Util::MakeHash(L""); }
			if (identifier) { idenHash = Util::MakeHash(identifier); } else { idenHash = Util::MakeHash(L""); }
			return std::make_tuple(modelHash, animHash, idenHash);
		}
	}

	void InstancingModelManager::Delete(const wchar_t* filePath, const AnimationClip* animationClip, const wchar_t* identifier) {
		//キーを作成
		std::tuple<std::size_t, std::size_t, std::size_t> key = CreateInstancingModelMapKey(filePath, animationClip, identifier);
		//削除
		delete m_instancingModelMap[key];
		m_instancingModelMap.erase(key);
	}

	GameObj::InstancingModel* InstancingModelManager::Load(
		int instanceMax,
		const wchar_t* filePath,
		const AnimationClip* animationClip,
		EnFbxUpAxis fbxUpAxis,
		EnFbxCoordinateSystem fbxCoordinate,
		const wchar_t* identifier,
		bool* return_isNewload 
	) {
		//キーを作成
		std::tuple<std::size_t, std::size_t, std::size_t> key = CreateInstancingModelMapKey(filePath, animationClip, identifier);

		//既に登録されてないか?
		if (m_instancingModelMap.count(key) > 0) {
			//登録されてたらマップから取得
			if (return_isNewload) { *return_isNewload = false; }
			return m_instancingModelMap[key];
		}
		else {
			//新規読み込み
			if (return_isNewload) { *return_isNewload = true; }
			m_instancingModelMap[key] = new GameObj::InstancingModel(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
			return m_instancingModelMap[key];
		}
	}

namespace GameObj {

	void InstancingModel::PreLoopUpdate() {
		//ループ前にインスタンス数のリセット
		//for (int i = 0; i < m_instanceIndex; i++) {//m_instanceMax
		//	m_insWatchers[i].reset();
		//}
		m_instanceIndex = 0;
	}

	//初期化
	void InstancingModel::Init(int instanceMax,
		const wchar_t* filePath,
		const AnimationClip* animationClip,
		EnFbxUpAxis fbxUpAxis,
		EnFbxCoordinateSystem fbxCoordinate
	) {
		//アニメーションクリップのコピー
		AnimationClip* animPtr = nullptr;
		if (animationClip) { 
			m_animationClip = *animationClip; 
			animPtr = &m_animationClip;
		}
		//モデル初期化
		m_model.Init(filePath, animPtr, animPtr ? 1 : 0, fbxUpAxis, fbxCoordinate);
		//ワールド行列を計算させない
		m_model.GetSkinModel().SetIsCalcWorldMatrix(false);
		//視錐台カリングを無効化(こちら側でやる)
		m_isFrustumCull = m_model.GetSkinModel().GetIsFrustumCulling();//こちら側に設定
		m_model.GetSkinModel().SetIsFrustumCulling(false);
		//インスタンシング用頂点シェーダをロード
		if (!m_s_isShaderLoaded) {
			D3D_SHADER_MACRO macros[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
			m_s_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "INSTANCING", macros);
			m_s_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			m_s_vsSkinShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, "INSTANCING", macros);
			m_s_vsZSkinShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			m_s_isShaderLoaded = true;
		}
		//インスタンシング用頂点シェーダをセット
		m_model.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				if (mat->GetIsSkining()) {
					//スキンモデル
					mat->SetVS(&m_s_vsSkinShader);
					mat->SetVSZ(&m_s_vsZSkinShader);
				}
				else {
					//スキンじゃないモデル
					mat->SetVS(&m_s_vsShader);
					mat->SetVSZ(&m_s_vsZShader);
				}
			}
		);
		//カリング前にやる処理を設定
		m_model.GetSkinModel().SetPreCullingFunction(
			[&](SkinModel*) {
				m_instanceIndex = max(0, m_instanceIndex);//0以下にはならない

				//描画判定
				int drawNum = 0;
				for (int i = 0; i < m_instanceIndex; i++) {
					//描画しない
					//if (m_insWatchers[i].expired() || !m_insWatchers[i].lock()->GetIsDraw()) { m_drawInstanceMask[i] = false; continue; }
					if (!m_isDraw[i]) { m_drawInstanceMask[i] = false; continue; }

					//視錐台カリング
					if (m_isFrustumCull) {
						if (!FrustumCulling::AABBTest(GetMainCamera(), m_minAABB[i], m_maxAABB[i])) {
							//描画しない
							m_drawInstanceMask[i] = false;
							continue;
						}
					}
					//描画する
					m_drawInstanceMask[i] = true;
					m_worldMatrixSB.GetData()[drawNum] = m_worldMatrixCache[i];
					m_worldMatrixSBOld.GetData()[drawNum] = m_worldMatrixOldCache[i];
					drawNum++;
				}

				//描画インスタンス数の設定
				m_model.GetSkinModel().SetInstanceNum(drawNum);
				m_instanceDrawNum = drawNum;
			}
		);
		//描画前にやる処理を設定
		m_model.GetSkinModel().SetPreDrawFunction(
			L"DW_InstancingModelFunc",
			[&](SkinModel*) {
				//ストラクチャーバッファの更新
				m_worldMatrixSB.UpdateSubresource();
				m_worldMatrixSBOld.UpdateSubresource();

				//シェーダーリソースにワールド行列をセット
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrix, 1, m_worldMatrixSB.GetAddressOfSRV());
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrixOld, 1, m_worldMatrixSBOld.GetAddressOfSRV());

				//IInstanceDataの処理実行
				for (auto& IID : m_instanceData) {
					IID.second->PreDraw(m_instanceIndex, m_instanceDrawNum, m_drawInstanceMask); 
				}

				//設定されている処理実行
				if (m_preDrawFunc) { m_preDrawFunc(); }
			}
		);

		//最大インスタンス数設定
		SetInstanceMax(instanceMax);
	}

	void InstancingModel::Release() {
		m_instanceMax = 0;
		m_instanceIndex = 0; m_instanceDrawNum = 0;

		//インスタンシング用リソースの開放
		m_worldMatrixSB.Release();
		m_worldMatrixSBOld.Release();

		m_isDraw.reset();
		m_drawInstanceMask.reset();
		m_minAABB.reset(); m_maxAABB.reset();
		m_worldMatrixCache.reset();
		m_worldMatrixOldCache.reset();

		//m_insWatchers.reset();

		m_instanceData.clear();
	}

	void InstancingModel::SetInstanceMax(int instanceMax) {
		//IInstanceDataの処理実行
		for (auto& IID : m_instanceData) {
			IID.second->SetInstanceMax(instanceMax);
		}

		//設定最大数が現在以下なら数だけ変更
		if (m_instanceMax >= instanceMax) {
			m_instanceMax = instanceMax;
			return;
		}

		//いろいろ再確保
		Release();
		
		//最大インスタンス数
		m_instanceMax = instanceMax;
		
		//視錐台カリング用
		m_isDraw = std::make_unique<bool[]>(m_instanceMax);
		m_drawInstanceMask = std::make_unique<bool[]>(m_instanceMax);
		m_minAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_maxAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_worldMatrixCache = std::make_unique<CMatrix[]>(m_instanceMax);
		m_worldMatrixOldCache = std::make_unique<CMatrix[]>(m_instanceMax);

		//インスタンスたちを監視する
		//m_insWatchers = std::make_unique<std::weak_ptr<InstanceWatcher>[]>(m_instanceMax);
		
		//StructuredBufferの確保		
		m_worldMatrixSB.Init(m_instanceMax);
		m_worldMatrixSBOld.Init(m_instanceMax);
	}
	
	CInstancingModelRender::CInstancingModelRender(bool isRegister) : IQSGameObject(isRegister) {
		//m_watcher = std::make_shared<InstanceWatcher>();
		//m_watcher->Watch(this);
	}
	CInstancingModelRender::~CInstancingModelRender() {
		//m_watcher->Watch(nullptr);
	}
	InstancingModelManager CInstancingModelRender::m_s_instancingModelManager;
}
}