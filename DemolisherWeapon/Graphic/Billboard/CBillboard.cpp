#include "DWstdafx.h"
#include "CBillboard.h"

namespace DemolisherWeapon {
namespace GameObj {
	CBillboard::CBillboard()
	{
	}
	CBillboard::~CBillboard()
	{
	}

	void CBillboard::Init(std::experimental::filesystem::path fileName, int instancingNum) {
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

		//������
		Init(tex, instancingNum, fileName.c_str());

		//�e�N�X�`���A�����[�X
		if (tex) {
			tex->Release();
		}
	}
	void CBillboard::Init(ID3D11ShaderResourceView* srv, int instancingNum, const wchar_t* identifiers) {
		//�C���X�^���V���O�`�悩?
		m_isIns = instancingNum > 1 && identifiers ? true : false;

		//�r���{�[�h���f���ǂݍ���
		if (m_isIns) {
			m_insModel.Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, &identifiers);
		}
		else {
			m_model.Init(L"Preset/modelData/billboard.cmo");
		}

		//�e�N�X�`���K��
		GameObj::CSkinModelRender* modelPtr = &m_model;
		if (m_isIns) {
			modelPtr = &m_insModel.GetInstancingModel()->GetModelRender();
		}
		modelPtr->GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(srv);
			}
		);

		//����������
		m_isInit = true;
	}

	void CBillboard::PostLoopUpdate() {
		if (!m_isInit) { return; }

		//�ʒu���X�V
		CQuaternion q = GetBillboardQuaternion();
		q.Multiply(m_rot);
		if (m_isIns) {
			m_insModel.SetRot(q);
		}
		else {
			m_model.SetRot(q);
		}
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
}