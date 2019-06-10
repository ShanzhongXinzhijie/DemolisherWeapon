#include "DWstdafx.h"
#include "CBillboard.h"

namespace DemolisherWeapon {
//namespace GameObj {
	
	//(インスタンシング)SRT行列を保存する処理

	void CBillboard::InstancingSRTRecorder::Reset(int instancingMaxNum) {
		m_instanceMax = instancingMaxNum;
		m_SRTMatrix = std::make_unique<CMatrix[]>(m_instanceMax);
		m_maxScale = std::make_unique<float[]>(m_instanceMax);
	}
	CBillboard::InstancingSRTRecorder::InstancingSRTRecorder(int instancingMaxNum) {
		Reset(instancingMaxNum);
	}
	void CBillboard::InstancingSRTRecorder::AddDrawInstance(int instanceNum, const CMatrix& SRTMatrix, const CVector3& scale) {
		m_SRTMatrix[instanceNum] = SRTMatrix;
		m_maxScale[instanceNum] = max(scale.x,max(scale.y, scale.z));
	}
	void CBillboard::InstancingSRTRecorder::SetInstanceMax(int instanceMax) {
		if (instanceMax > m_instanceMax) {
			Reset(instanceMax);
		}
	}

	//シャドウマップ描画時の式

	CBillboard::ShodowWorldMatrixCalcer::ShodowWorldMatrixCalcer(CBillboard* model) : m_ptrBillboard(model) {
		m_ptrModel = &m_ptrBillboard->GetModel().GetSkinModel();
	}
	void CBillboard::ShodowWorldMatrixCalcer::PreDraw() {
		//現在のワールド行列の保存
		m_worldMatrix = m_ptrModel->GetWorldMatrix();
	}
	void CBillboard::ShodowWorldMatrixCalcer::PreModelDraw() {
		//新たなワールド行列に更新
		//ポジションをずらす...カメラの前方向に
		m_ptrModel->UpdateBillBoardMatrix(GetMainCamera()->GetFront()*m_ptrBillboard->GetMaxScale());
	}
	void CBillboard::ShodowWorldMatrixCalcer::PostDraw() {
		//ワールド行列を戻す
		m_ptrModel->SetWorldMatrix(m_worldMatrix);
	}

	//(インスタンシング)シャドウマップ描画時の式

	CBillboard::ShodowWorldMatrixCalcerInstancing::ShodowWorldMatrixCalcerInstancing(GameObj::InstancingModel* model, InstancingSRTRecorder* insSRT)
	: m_ptrModel(model), m_ptrInsSRT(insSRT){
		m_instancesNum = m_ptrModel->GetInstanceMax();
		m_worldMatrix = std::make_unique<CMatrix[]>(m_instancesNum);
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PreDraw() {
		//最大インスタンス数の増加に対応
		if (m_instancesNum < m_ptrModel->GetInstanceMax()) {
			m_instancesNum = m_ptrModel->GetInstanceMax();
			m_worldMatrix.reset();
			m_worldMatrix = std::make_unique<CMatrix[]>(m_instancesNum);
		}
		//現在のワールド行列の保存
		const auto& mats = m_ptrModel->GetWorldMatrix();
		int max = m_ptrModel->GetDrawInstanceNum();
		for (int i = 0; i < max; i++) {
			m_worldMatrix[i] = mats[i];
		}
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PreModelDraw() {
		//新たなワールド行列に更新
		//ポジションをずらす...カメラの前方向に
		m_ptrModel->UpdateBillBoardMatrix(m_ptrInsSRT->GetSRTMatrix().get(), m_ptrInsSRT->GetMaxScale().get());
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PostDraw() {
		//ワールド行列を戻す
		m_ptrModel->SetUpdateDrawWorldMatrix(m_worldMatrix.get());
	}
	
	//ビルボード

	void CBillboard::Init(std::experimental::filesystem::path fileName, int instancingNum) {
		//テクスチャ読み込み
		ID3D11ShaderResourceView* tex = nullptr;
		HRESULT hr;
		if (wcscmp(fileName.extension().c_str(), L".dds") == 0) {
			hr = DirectX::CreateDDSTextureFromFile(GetGraphicsEngine().GetD3DDevice(), fileName.c_str(), nullptr, &tex);
		}
		else {
			hr = DirectX::CreateWICTextureFromFile(GetGraphicsEngine().GetD3DDevice(), fileName.c_str(), nullptr, &tex);
		}
		if (FAILED(hr)) {
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "CBillboard::Init()の画像読み込みに失敗。\nファイルパスあってますか？\n%ls\n", fileName.c_str());
			DemolisherWeapon::Error::Box(message);
#endif
			return;
		}

		//初期化
		Init(tex, instancingNum, fileName.c_str());

		//テクスチャ、リリース
		if (tex) {
			tex->Release();
		}
	}
	void CBillboard::Init(ID3D11ShaderResourceView* srv, int instancingNum, const wchar_t* identifiers, bool isSetIInstancesDataAndShadowPrePost) {
		//インスタンシング描画か?
		m_isIns = instancingNum > 1 && identifiers ? true : false;

		//ビルボードモデル読み込み
		if (m_isIns) {
			m_insModel = std::make_unique<GameObj::CInstancingModelRender>();
			m_insModel->Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, &identifiers);
		}
		else {
			m_model = std::make_unique<GameObj::CSkinModelRender>();
			m_model->Init(L"Preset/modelData/billboard.cmo");
		}
		//m_model.SetIsDraw(false);

		//テクスチャ適応
		GameObj::CSkinModelRender* modelPtr = m_model.get();
		if (m_isIns) {
			modelPtr = &m_insModel->GetInstancingModel()->GetModelRender();
		}
		modelPtr->GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(srv);
				mat->SetIsUseTexZShader(true);//Z値出力シェーダでテクスチャを使用
			}
		);

		//ビルボードであると設定
		modelPtr->GetSkinModel().SetIsBillboard(true);
		//シャドウマップの描画時に面を反転させない
		modelPtr->SetIsShadowDrawReverse(false);

		if (isSetIInstancesDataAndShadowPrePost) {
			//インスタンシング用のクラス設定
			InstancingSRTRecorder* insSRT = nullptr;
			if (m_isIns) {
				if (!m_insModel->GetInstancingModel()->GetIInstanceData()) {
					//新規作成
					m_insModel->GetInstancingModel()->SetIInstanceData(std::make_unique<InstancingSRTRecorder>(m_insModel->GetInstancingModel()->GetInstanceMax()));
				}
				//既存のもの使う
				insSRT = dynamic_cast<InstancingSRTRecorder*>(m_insModel->GetInstancingModel()->GetIInstanceData());
				insSRT->SetInstanceMax(m_insModel->GetInstancingModel()->GetInstanceMax());
			}
			//シャドウマップ描画時に実行する処理を設定
			if (!modelPtr->GetShadowMapPrePost()) {
				//ビルボードのものを設定
				if (m_isIns) {
					modelPtr->SetShadowMapPrePost(std::make_unique<ShodowWorldMatrixCalcerInstancing>(m_insModel->GetInstancingModel(), insSRT));
				}
				else {
					modelPtr->SetShadowMapPrePost(std::make_unique<ShodowWorldMatrixCalcer>(this));
				}
			}
		}

		//初期化完了
		m_isInit = true;
	}

//}
}