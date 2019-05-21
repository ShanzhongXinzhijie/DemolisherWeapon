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
		//�r���{�[�h���f���ǂݍ���
		m_model.Init(L"Preset/modelData/billboard.cmo");

		//�e�N�X�`���ǂݍ���
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
			sprintf_s(message, "CBillboard::Init()�̉摜�ǂݍ��݂Ɏ��s�B\n�t�@�C���p�X�����Ă܂����H\n%ls\n", fileName.c_str());
			DemolisherWeapon::Error::Box(message);
#endif
			return;
		}

		//�e�N�X�`���K��
		m_model.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(tex);
			}
		);

		//�e�N�X�`���A�����[�X
		if (tex) {
			tex->Release();
		}

		m_isInit = true;
	}

	void CBillboard::Update() {
		if (!m_isInit) { return; }

		//�ʒu���X�V
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