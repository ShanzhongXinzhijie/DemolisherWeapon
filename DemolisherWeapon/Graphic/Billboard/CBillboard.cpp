#include "DWstdafx.h"
#include "CBillboard.h"

namespace DemolisherWeapon {
//namespace GameObj {
	CBillboard::CBillboard()
	{
	}
	CBillboard::~CBillboard()
	{
	}

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
	void CBillboard::Init(ID3D11ShaderResourceView* srv, int instancingNum, const wchar_t* identifiers) {
		//インスタンシング描画か?
		m_isIns = instancingNum > 1 && identifiers ? true : false;

		//ビルボードモデル読み込み
		if (m_isIns) {
			m_insModel.Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, &identifiers);
		}
		else {
			m_model.Init(L"Preset/modelData/billboard.cmo");
		}

		//テクスチャ適応
		GameObj::CSkinModelRender* modelPtr = &m_model;
		if (m_isIns) {
			modelPtr = &m_insModel.GetInstancingModel()->GetModelRender();
		}
		modelPtr->GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(srv);
			}
		);

		//ビルボードであると設定
		modelPtr->GetSkinModel().SetIsBillboard(true);

		//初期化完了
		m_isInit = true;
	}

	//void CBillboard::PostLoopUpdate() {
	//	if (!m_isInit) { return; }

	//	//位置等更新
	//	CQuaternion q = GetBillboardQuaternion();
	//	q.Multiply(m_rot);
	//	if (m_isIns) {
	//		m_insModel.SetRot(q);
	//	}
	//	else {
	//		m_model.SetRot(q);
	//	}
	//}

	CQuaternion CBillboard::GetBillboardQuaternion() {
		CQuaternion q;
		q.SetRotation(GetBillboardMatrix());
		return q;
	}

	CMatrix CBillboard::GetBillboardMatrix() {
		CMatrix m = GetMainCamera()->GetViewMatrix();
		m.Inverse();
		return m;
	}
//}
}