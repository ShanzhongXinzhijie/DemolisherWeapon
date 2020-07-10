#include "DWstdafx.h"
#include "CSkybox.h"

namespace DemolisherWeapon {
namespace GameObj {

	CSkybox::CSkybox(const wchar_t* filePass, float size, bool isSetAmbientCube, const CVector3& ambientScale) {
		Init(filePass, size, isSetAmbientCube);
	}
	
	void CSkybox::Init(const wchar_t* filePass, float size, bool isSetAmbientCube, const CVector3& ambientScale)
	{
		if (m_isInit) { return; }

		//�e�N�X�`��
		ID3D11ShaderResourceView* texSRV = nullptr;
		TextueData texData = CreateTexture(filePass);
		if (!texData.isLoaded()) {
			Error::Box("CSkybox�̃e�N�X�`���ǂݍ��݂Ɏ��s���܂���");
			return;
		}
		texSRV = texData.textureView.Get(); texSRV->AddRef();

		//���f��
		m_skyModel.Init(L"Preset/modelData/sky.cmo");
		
		//�V�F�[�_
		D3D_SHADER_MACRO macros[] = { "SKY_CUBE", "1", "MOTIONBLUR", "1", NULL, NULL };
		m_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "SKY_CUBE", macros);
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "SKY_CUBE", macros);

		//���f���Ƀe�N�X�`���ƃV�F�[�_�ݒ�
		m_skyModel.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(texSRV);
				mat->SetAlbedoScale(ambientScale);
				mat->SetVS(&m_vsShader);
				mat->SetPS(&m_psShader);
				mat->SetLightingEnable(false);
			}
		);

		//���L���[�u�}�b�v��ݒ�
		if (isSetAmbientCube) {
			SetAmbientCubeMap(texSRV, ambientScale);
		}

		if (texSRV) { texSRV->Release(); }

		//�ݒ�
		m_skyModel.SetDrawPriority(DRAW_PRIORITY_MAX-1);
		m_skyModel.SetIsMostDepth(true);
		m_skyModel.SetIsShadowCaster(false);
		
		//�傫��
		//TODO �����J�������g���ꍇ�A���󂾂�Far�����킹��K�v������
		if (size < 0.0f) { size = std::floor(GetMainCamera()->GetFar() * (1.0f/sqrt(3.0f))); }
		m_skyModel.SetScale({ size / 50.0f, size / 50.0f, size / 50.0f });

		//�`��O������ݒ�
		m_skyModel.GetSkinModel().SetPreCullingFunction(
			[&](SkinModel* model) {
				m_skyModel.SetPos(GetMainCamera()->GetPos());
				m_skyModel.RefreshWorldMatrix();
			}
		);

		m_isInit = true;
	}
}
}