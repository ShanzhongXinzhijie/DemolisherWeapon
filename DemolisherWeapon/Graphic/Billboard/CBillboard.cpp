#include "DWstdafx.h"
#include "CBillboard.h"

namespace DemolisherWeapon {
	CBillboard::CBillboard()
	{
	}
	CBillboard::~CBillboard()
	{
	}

	void CBillboard::Init(std::experimental::filesystem::path fileName) {
		//ビルボードモデル読み込み
		m_model.Init(L"Preset/modelData/billboard.cmo");

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

		//テクスチャ適応
		m_model.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(tex);
			}
		);

		//テクスチャ、リリース
		if (tex) {
			tex->Release();
		}

		m_isInit = true;
	}

	void CBillboard::Update() {
		if (!m_isInit) { return; }

		//位置等更新
		CQuaternion q = GetBillboardQuaternion();
		q.Multiply(m_rot);
		m_model.SetRot(q);
	}

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
}