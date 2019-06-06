#include "DWstdafx.h"
#include "CBillboard.h"

namespace DemolisherWeapon {
//namespace GameObj {
	
	CBillboard::InstancingSRTRecorder::InstancingSRTRecorder(int instancingMaxNum) {
		m_SRTMatrix = std::make_unique<CMatrix[]>(instancingMaxNum);
	}
	void CBillboard::InstancingSRTRecorder::AddDrawInstance(int instanceNum, const CMatrix& SRTMatrix) {
		m_SRTMatrix[instanceNum] = SRTMatrix;
	}

	CBillboard::ShodowWorldMatrixCalcer::ShodowWorldMatrixCalcer(SkinModel* model) : m_ptrModel(model) {
	}
	void CBillboard::ShodowWorldMatrixCalcer::PreDraw() {
		//���݂̃��[���h�s��̕ۑ�
		m_worldMatrix = m_ptrModel->GetWorldMatrix();
		//TODO �[�x�l�o�C�A�X
		//�ԓ��͂����������ƂłȂ��̂�DEPTH_BIAS_D32_FLOAT
		//m_depthBias = m_ptrModel->GetDepthBias();
		//m_ptrModel->SetDepthBias(m_depthBias + );
	}
	void CBillboard::ShodowWorldMatrixCalcer::PreModelDraw() {
		//�V���ȃ��[���h�s��ɍX�V
		m_ptrModel->UpdateBillBoardMatrix();
	}
	void CBillboard::ShodowWorldMatrixCalcer::PostDraw() {
		//���[���h�s���߂�
		m_ptrModel->SetWorldMatrix(m_worldMatrix);
		//�[�x�l�o�C�A�X�߂�
		//m_ptrModel->SetDepthBias(m_depthBias);
	}

	CBillboard::ShodowWorldMatrixCalcerInstancing::ShodowWorldMatrixCalcerInstancing(GameObj::InstancingModel* model, InstancingSRTRecorder* insSRT)
	: m_ptrModel(model), m_ptrInsSRT(insSRT){
		m_instancesNum = m_ptrModel->GetInstanceMax();//TODO �r���ύX�s��
		m_worldMatrix = std::make_unique<CMatrix[]>(m_instancesNum);
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PreDraw() {
		//���݂̃��[���h�s��̕ۑ�
		const auto& mats = m_ptrModel->GetWorldMatrix();
		int max = m_ptrModel->GetDrawInstanceNum();
		for (int i = 0; i < max; i++) {
			m_worldMatrix[i] = mats[i];
		}
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PreModelDraw() {
		//�V���ȃ��[���h�s��ɍX�V
		m_ptrModel->UpdateBillBoardMatrix(m_ptrInsSRT->GetSRTMatrix().get());
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PostDraw() {
		//���[���h�s���߂�
		m_ptrModel->SetUpdateDrawWorldMatrix(m_worldMatrix.get());
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
	void CBillboard::Init(ID3D11ShaderResourceView* srv, int instancingNum, const wchar_t* identifiers, bool isSetIInstancesDataAndShadowPrePost) {
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
				mat->SetIsUseTexZShader(true);//Z�l�o�̓V�F�[�_�Ńe�N�X�`�����g�p
			}
		);

		//�r���{�[�h�ł���Ɛݒ�
		modelPtr->GetSkinModel().SetIsBillboard(true);
		//�V���h�E�}�b�v�̕`�掞�ɖʂ𔽓]�����Ȃ�
		modelPtr->SetIsShadowDrawReverse(false);

		if (isSetIInstancesDataAndShadowPrePost) {
			//�C���X�^���V���O�p�̃N���X�ݒ�
			InstancingSRTRecorder* insSRT = nullptr;
			if (m_isIns) {
				if (!m_insModel.GetInstancingModel()->GetIInstanceData()) {
					//�V�K�쐬
					m_insModel.GetInstancingModel()->SetIInstanceData(std::make_unique<InstancingSRTRecorder>(m_insModel.GetInstancingModel()->GetInstanceMax()));
				}
				//�����̂��̎g��
				insSRT = dynamic_cast<InstancingSRTRecorder*>(m_insModel.GetInstancingModel()->GetIInstanceData());
			}
			//�V���h�E�}�b�v�`�掞�Ɏ��s���鏈����ݒ�
			if (!modelPtr->GetShadowMapPrePost()) {
				//�r���{�[�h�̂��̂�ݒ�
				if (m_isIns) {
					modelPtr->SetShadowMapPrePost(std::make_unique<ShodowWorldMatrixCalcerInstancing>(m_insModel.GetInstancingModel(), insSRT));
				}
				else {
					modelPtr->SetShadowMapPrePost(std::make_unique<ShodowWorldMatrixCalcer>(&modelPtr->GetSkinModel()));
				}
			}
		}

		//����������
		m_isInit = true;
	}

//}
}