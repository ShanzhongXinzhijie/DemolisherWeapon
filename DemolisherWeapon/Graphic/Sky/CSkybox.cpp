#include "DWstdafx.h"
#include "CSkybox.h"

namespace DemolisherWeapon {
namespace GameObj {

	CSkybox::CSkybox(const wchar_t* filePass, float size, bool isSetAmbientCube) {
		Init(filePass, size, isSetAmbientCube);
	}
	
	void CSkybox::Init(const wchar_t* filePass, float size, bool isSetAmbientCube)
	{
		if (m_isInit) { return; }

		//�e�N�X�`��
		ID3D11ShaderResourceView* tex = nullptr;
		HRESULT hr = DirectX::CreateDDSTextureFromFile(GetEngine().GetGraphicsEngine().GetD3DDevice(), filePass, nullptr, &tex);
		if (FAILED(hr)) {
			Error::Box("CSkybox�̃e�N�X�`���ǂݍ��݂Ɏ��s���܂���");
			return;
		}

		//���f��
		m_skyModel.Init(L"Preset/modelData/sky.cmo");
		
		//�V�F�[�_
		D3D_SHADER_MACRO macros[] = { "SKY_CUBE", "1", NULL, NULL };
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "SKY_CUBE", macros);

		//���f���Ƀe�N�X�`���ƃV�F�[�_�ݒ�
		m_skyModel.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetAlbedoTexture(tex);
				mat->SetPS(&m_psShader);
				mat->SetLightingEnable(false);
			}
		);

		//���L���[�u�}�b�v��ݒ�
		if (isSetAmbientCube) {
			SetAmbientCubeMap(tex, CVector3::One());
		}

		if (tex) { tex->Release(); }

		//�ݒ�
		m_skyModel.SetDrawPriority(DRAW_PRIORITY_MAX-1);
		m_skyModel.SetIsMostDepth(true);
		m_skyModel.SetIsShadowCaster(false);
		
		//�傫��
		if (size < 0.0f) { size = std::floor(GetMainCamera()->GetFar() * (1.0f/sqrt(3.0f))); }
		m_skyModel.SetScale({ size / 50.0f, size / 50.0f, size / 50.0f });

		m_isInit = true;
	}

	CSkybox::~CSkybox()
	{
	}

	void CSkybox::Update() {
		if (!m_isInit) { return; }
		m_skyModel.SetPos(GetMainCamera()->GetPos());
	}
}
}