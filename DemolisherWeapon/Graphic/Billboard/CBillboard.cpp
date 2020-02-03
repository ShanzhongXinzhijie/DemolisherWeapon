#include "DWstdafx.h"
#include "CBillboard.h"
#include "Graphic/Factory/TextureFactory.h"

namespace DemolisherWeapon {	

	void CBillboard::Init(const wchar_t* fileName, int instancingNum, bool isBillboardShader) {
		//�t�@�N�g���Ńe�N�X�`���ǂݍ���
		ID3D11ShaderResourceView* tex = nullptr;
		const TextureFactory::TextueData* texdata = nullptr;
		if (!TextureFactory::GetInstance().Load(fileName, nullptr, &tex, &texdata)) {
			//���s
			return;
		}

		//�A�X�y�N�g��Z�o
		m_aspect = (float)texdata->width / texdata->height;

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
			std::wstring_view string[1]; string[0] = identifiers;
			m_insModel->Init(instancingNum, L"Preset/modelData/billboard.cmo", nullptr, 0, enFbxUpAxisZ, enFbxRightHanded, string);
		}
		else {
			m_model = std::make_unique<GameObj::CSkinModelRender>();
			m_model->Init(L"Preset/modelData/billboard.cmo");
		}

		if (!m_s_isShaderLoaded && isBillboardShader) {
			//�V�F�[�_�ǂݍ���
			{
				//�C���X�^���V���O�p�V�F�[�_
				D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enInstancing].Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
				m_s_vsZShader[enInstancing].Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
			}
			{
				D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enNormal].Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
				m_s_vsZShader[enNormal].Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
			}
			m_s_isShaderLoaded = true;
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
					mat->SetVS(&m_s_vsShader[m_isIns ? enInstancing : enNormal]);
					mat->SetVSZ(&m_s_vsZShader[m_isIns ? enInstancing : enNormal]);
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