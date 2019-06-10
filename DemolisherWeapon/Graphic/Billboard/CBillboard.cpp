#include "DWstdafx.h"
#include "CBillboard.h"

namespace DemolisherWeapon {
//namespace GameObj {
	
	//(�C���X�^���V���O)SRT�s���ۑ����鏈��

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

	//�V���h�E�}�b�v�`�掞�̎�

	CBillboard::ShodowWorldMatrixCalcer::ShodowWorldMatrixCalcer(CBillboard* model) : m_ptrBillboard(model) {
		m_ptrModel = &m_ptrBillboard->GetModel().GetSkinModel();
	}
	void CBillboard::ShodowWorldMatrixCalcer::PreDraw() {
		//���݂̃��[���h�s��̕ۑ�
		m_worldMatrix = m_ptrModel->GetWorldMatrix();
	}
	void CBillboard::ShodowWorldMatrixCalcer::PreModelDraw() {
		//�V���ȃ��[���h�s��ɍX�V
		//�|�W�V���������炷...�J�����̑O������
		m_ptrModel->UpdateBillBoardMatrix(GetMainCamera()->GetFront()*m_ptrBillboard->GetMaxScale());
	}
	void CBillboard::ShodowWorldMatrixCalcer::PostDraw() {
		//���[���h�s���߂�
		m_ptrModel->SetWorldMatrix(m_worldMatrix);
	}

	//(�C���X�^���V���O)�V���h�E�}�b�v�`�掞�̎�

	CBillboard::ShodowWorldMatrixCalcerInstancing::ShodowWorldMatrixCalcerInstancing(GameObj::InstancingModel* model, InstancingSRTRecorder* insSRT)
	: m_ptrModel(model), m_ptrInsSRT(insSRT){
		m_instancesNum = m_ptrModel->GetInstanceMax();
		m_worldMatrix = std::make_unique<CMatrix[]>(m_instancesNum);
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PreDraw() {
		//�ő�C���X�^���X���̑����ɑΉ�
		if (m_instancesNum < m_ptrModel->GetInstanceMax()) {
			m_instancesNum = m_ptrModel->GetInstanceMax();
			m_worldMatrix.reset();
			m_worldMatrix = std::make_unique<CMatrix[]>(m_instancesNum);
		}
		//���݂̃��[���h�s��̕ۑ�
		const auto& mats = m_ptrModel->GetWorldMatrix();
		int max = m_ptrModel->GetDrawInstanceNum();
		for (int i = 0; i < max; i++) {
			m_worldMatrix[i] = mats[i];
		}
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PreModelDraw() {
		//�V���ȃ��[���h�s��ɍX�V
		//�|�W�V���������炷...�J�����̑O������
		m_ptrModel->UpdateBillBoardMatrix(m_ptrInsSRT->GetSRTMatrix().get(), m_ptrInsSRT->GetMaxScale().get());
	}
	void CBillboard::ShodowWorldMatrixCalcerInstancing::PostDraw() {
		//���[���h�s���߂�
		m_ptrModel->SetUpdateDrawWorldMatrix(m_worldMatrix.get());
	}
	
	//�r���{�[�h

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
			m_insModel = std::make_unique<GameObj::CInstancingModelRender>();
			m_insModel->Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, &identifiers);
		}
		else {
			m_model = std::make_unique<GameObj::CSkinModelRender>();
			m_model->Init(L"Preset/modelData/billboard.cmo");
		}
		//m_model.SetIsDraw(false);

		//�e�N�X�`���K��
		GameObj::CSkinModelRender* modelPtr = m_model.get();
		if (m_isIns) {
			modelPtr = &m_insModel->GetInstancingModel()->GetModelRender();
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
				if (!m_insModel->GetInstancingModel()->GetIInstanceData()) {
					//�V�K�쐬
					m_insModel->GetInstancingModel()->SetIInstanceData(std::make_unique<InstancingSRTRecorder>(m_insModel->GetInstancingModel()->GetInstanceMax()));
				}
				//�����̂��̎g��
				insSRT = dynamic_cast<InstancingSRTRecorder*>(m_insModel->GetInstancingModel()->GetIInstanceData());
				insSRT->SetInstanceMax(m_insModel->GetInstancingModel()->GetInstanceMax());
			}
			//�V���h�E�}�b�v�`�掞�Ɏ��s���鏈����ݒ�
			if (!modelPtr->GetShadowMapPrePost()) {
				//�r���{�[�h�̂��̂�ݒ�
				if (m_isIns) {
					modelPtr->SetShadowMapPrePost(std::make_unique<ShodowWorldMatrixCalcerInstancing>(m_insModel->GetInstancingModel(), insSRT));
				}
				else {
					modelPtr->SetShadowMapPrePost(std::make_unique<ShodowWorldMatrixCalcer>(this));
				}
			}
		}

		//����������
		m_isInit = true;
	}

//}
}