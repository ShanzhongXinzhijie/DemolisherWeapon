#include "DWstdafx.h"
#include "CBillboard.h"
#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {	

	void CBillboard::Init(const wchar_t* fileName, int instancingNum, bool isBillboardShader) {
		//�t�@�N�g���Ńe�N�X�`���ǂݍ���
		ID3D11ShaderResourceView* tex = nullptr;
		TextureFactory::GetInstance().Load(fileName, nullptr, &tex);

		//�r���{�[�h������
		Init(tex, instancingNum, fileName, isBillboardShader);

		//�e�N�X�`���A�����[�X
		if (tex) { tex->Release(); }
	}
	void CBillboard::Init(ID3D11ShaderResourceView* srv, int instancingNum, const wchar_t* identifiers, bool isBillboardShader) {
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

		if (isBillboardShader) {
			//�V�F�[�_�ǂݍ���
			if (m_isIns) {
				//�C���X�^���V���O�p�V�F�[�_
				D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
				m_vsShader.Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
				m_vsZShader.Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
			}
			else {
				D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
				m_vsShader.Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
				m_vsZShader.Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
			}
		}

		//CSkinModelRender
		GameObj::CSkinModelRender* modelPtr = m_model.get();
		if (m_isIns) { modelPtr = &m_insModel->GetInstancingModel()->GetModelRender(); }

		//���낢��ݒ�
		modelPtr->GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(srv);
				mat->SetIsUseTexZShader(true);//Z�l�o�̓V�F�[�_�Ńe�N�X�`�����g�p
				if (isBillboardShader) {
					mat->SetVS(&m_vsShader);
					mat->SetVSZ(&m_vsZShader);
				}
			}
		);

		//�o�E���f�B���O�{�b�N�X�̐ݒ�
		float size = sqrt(CMath::Square(0.5f) + CMath::Square(0.5f));
		modelPtr->GetSkinModel().SetBoundingBox({ -size,-size,-size }, { size,size,size });

		//�V���h�E�}�b�v�̕`�掞�ɖʂ𔽓]�����Ȃ�
		modelPtr->SetIsShadowDrawReverse(false);

		//����������
		m_isInit = true;
	}

}