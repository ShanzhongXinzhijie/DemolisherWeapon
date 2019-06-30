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
		const wchar_t* identifier
	) {
		//キーを作成
		std::tuple<std::size_t, std::size_t, std::size_t> key = CreateInstancingModelMapKey(filePath, animationClip, identifier);

		//既に登録されてないか?
		if (m_instancingModelMap.count(key) > 0) {
			//登録されてたらマップから取得
			return m_instancingModelMap[key];
		}
		else {
			//新規読み込み
			m_instancingModelMap[key] = new GameObj::InstancingModel(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
			return m_instancingModelMap[key];
		}
	}

namespace GameObj {

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
		D3D_SHADER_MACRO macros[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
		m_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "INSTANCING", macros);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
		m_vsSkinShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, "INSTANCING", macros);
		m_vsZSkinShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
		//インスタンシング用頂点シェーダをセット
		m_model.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
			if (mat->GetModelEffect()->GetIsSkining()) {
				//スキンモデル
				mat->SetVS(&m_vsSkinShader);
				mat->SetVSZ(&m_vsZSkinShader);
			}
			else {
				//スキンじゃないモデル
				mat->SetVS(&m_vsShader);
				mat->SetVSZ(&m_vsZShader);
			}
		}
		);
		//カリング前にやる処理を設定
		m_model.GetSkinModel().SetPreCullingFunction(
			[&](SkinModel*) {
				m_instanceIndex = max(0, m_instanceIndex);

				//視錐台カリング
				int drawNum = 0;
				for (int i = 0; i < m_instanceIndex; i++) {
					if (m_isFrustumCull) {
						if (!FrustumCulling::AABBTest(GetMainCamera(), m_minAABB[i], m_maxAABB[i])) {
							//描画しない
							m_drawInstanceMask[i] = false;
							continue;
						}
					}
					//描画する
					m_drawInstanceMask[i] = true;
					m_instancingWorldMatrix[drawNum] = m_worldMatrixCache[i];
					m_instancingWorldMatrixOld[drawNum] = m_worldMatrixOldCache[i];
					drawNum++;
				}

				//描画インスタンス数の設定
				m_model.GetSkinModel().SetInstanceNum(drawNum);
				m_instanceDrawNum = drawNum;
			}
		);
		//描画前にやる処理を設定
		m_model.GetSkinModel().SetPreDrawFunction(
			[&](SkinModel*) {
				//ストラクチャーバッファの更新
				GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
					m_worldMatrixSB, 0, NULL, m_instancingWorldMatrix.get(), 0, 0
				);
				GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
					m_worldMatrixSBOld, 0, NULL, m_instancingWorldMatrixOld.get(), 0, 0
				);

				//シェーダーリソースにワールド行列をセット
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrix, 1, &m_worldMatrixSRV);
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrixOld, 1, &m_worldMatrixSRVOld);

				//IInstanceDataの処理実行
				if (m_instanceData) { m_instanceData->PreDraw(m_instanceIndex, m_instanceDrawNum, m_drawInstanceMask); }

				//設定されている処理実行
				if (m_preDrawFunc) { m_preDrawFunc(); }
			}
		);

		//最大インスタンス数設定
		SetInstanceMax(instanceMax);
	}

	void InstancingModel::SetInstanceMax(int instanceMax) {
		//IInstanceDataの処理実行
		if (m_instanceData) { m_instanceData->SetInstanceMax(instanceMax); }

		//設定最大数が現在以下なら数だけ変更
		if (m_instanceMax >= instanceMax) {
			m_instanceMax = instanceMax;
			return;
		}

		//いろいろ再確保
		Release();
		
		m_instanceMax = instanceMax;
		
		//ワールド行列の確保
		m_instancingWorldMatrix = std::make_unique<CMatrix[]>(m_instanceMax);
		m_instancingWorldMatrixOld = std::make_unique<CMatrix[]>(m_instanceMax);
		//視錐台カリング用
		m_drawInstanceMask = std::make_unique<bool[]>(m_instanceMax);
		m_minAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_maxAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_worldMatrixCache = std::make_unique<CMatrix[]>(m_instanceMax);
		m_worldMatrixOldCache = std::make_unique<CMatrix[]>(m_instanceMax);
		
		//StructuredBufferの確保
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(CMatrix);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					
		desc.ByteWidth = static_cast<UINT>(stride * m_instanceMax);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_worldMatrixSB);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_worldMatrixSBOld);
		
		//ShaderResourceViewの確保
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_worldMatrixSB, &descSRV, &m_worldMatrixSRV);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_worldMatrixSBOld, &descSRV, &m_worldMatrixSRVOld);
	}
	
	InstancingModelManager CInstancingModelRender::m_s_instancingModelManager;
}
}