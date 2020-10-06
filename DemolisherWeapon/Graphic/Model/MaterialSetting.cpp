#include "DWstdafx.h"
#include "MaterialSetting.h"
#include "Graphic/Factory/TextureFactory.h"
#include "Graphic/Model/Model.h"

namespace DemolisherWeapon {

	void MaterialData::Init(bool isSkining, std::wstring_view name) {
		m_isSkining = isSkining;
		
		//���O�ݒ�
		GetDefaultMaterialSetting().SetMatrialName(name.data());

		//�V�F�[�_�쐬
		{
			//���_�V�F�[�_
			if (m_isSkining) {
				//�}�N�����Ƃɒ��_�V�F�[�_���쐬
				m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
				//Z�l�`��V�F�[�_���쐬
				m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);
			}
			else {
				//�}�N�����Ƃɒ��_�V�F�[�_���쐬
				m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
				//Z�l�`��V�F�[�_���쐬
				m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);
			}

			//�}�N�����ƂɃs�N�Z���V�F�[�_���쐬
			m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
			m_psTriPlanarMapShader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
			D3D_SHADER_MACRO macrosYOnly[] = { "Y_ONLY", "1", NULL, NULL };
			m_psTriPlanarMapShaderYOnly.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS, "Y_ONLY", macrosYOnly);

			//�}�N�����Ƃ�Z�l�`��s�N�Z���V�F�[�_���쐬
			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
			m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
			m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		}

		//�}�e���A���ݒ�(m_defaultMaterialSetting)�����������Ă��
		m_defaultMaterialSetting.Init(this);

		//�g�p����}�e���A���ݒ���Z�b�g
		m_ptrUseMaterialSetting = &m_defaultMaterialSetting;

		//�}�e���A���p�����[�^�[�̒萔�o�b�t�@
		if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
			ShaderUtil::CreateConstantBuffer(sizeof(MaterialParam), &m_materialParamCBDX11);
		}
		if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
			//TODO
			//m_materialParamCBDX12
		}
	}
	void MaterialData::InitAlbedoTexture(std::wstring_view path) {
		//�A���x�h
		const TextueData* return_textureData;
		if (TextureFactory::GetInstance().Load(path.data(), &return_textureData, true)) {
			//�e�N�X�`���Őݒ�
			m_defaultMaterialSetting.SetAlbedoTexture(*return_textureData);
		}
	}
	void MaterialData::InitAlbedoColor(const CVector3& rgb) {
		//�f�B�t���[�Y�J���[(���l)�Őݒ�
		m_defaultMaterialSetting.SetAlbedoScale({ rgb.x,rgb.y,rgb.z, 1.0f });
	}
	void MaterialData::InitNormalTexture(std::wstring_view path) {
		//�m�[�}��
		const TextueData* return_textureData;
		if (TextureFactory::GetInstance().Load(path.data(), &return_textureData, true))
		{
			//�e�N�X�`���Őݒ�			
			m_defaultMaterialSetting.SetNormalTexture(*return_textureData);
		}
	}
	void MaterialData::InitLightingTexture(std::wstring_view path) {
		//���C�e�B���O�p�����[�^
		const TextueData* return_textureData;
		if (TextureFactory::GetInstance().Load(path.data(), &return_textureData, true))
		{
			//�e�N�X�`���Őݒ�
			m_defaultMaterialSetting.SetLightingTexture(*return_textureData);
		}
	}	

	/*void MaterialSetting::Init(ModelEffect* modeleffect) {
		m_isInit = modeleffect;//���ƂȂ郂�f���G�t�F�N�g���L�^

		//������
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultPSZ();
		SetDefaultAlbedoTexture();
		SetDefaultNormalTexture();
		SetDefaultLightingTexture();

		//�ݒ�̃R�s�[
		SetMatrialName(modeleffect->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(modeleffect->GetDefaultMaterialSetting().GetMaterialParam());

		SetVS(modeleffect->GetDefaultMaterialSetting().GetVS());
		SetVSZ(modeleffect->GetDefaultMaterialSetting().GetVSZ());
		SetPS(modeleffect->GetDefaultMaterialSetting().GetPS());
		SetPSZ(modeleffect->GetDefaultMaterialSetting().GetPSZ());

		SetAlbedoTexture(modeleffect->GetDefaultMaterialSetting().GetAlbedoTexture());
		SetNormalTexture(modeleffect->GetDefaultMaterialSetting().GetNormalTexture());
		SetLightingTexture(modeleffect->GetDefaultMaterialSetting().GetLightingTexture());
		SetTranslucentTexture(modeleffect->GetDefaultMaterialSetting().GetTranslucentTexture());

		SetIsMotionBlur(modeleffect->GetDefaultMaterialSetting().GetIsMotionBlur());
		SetIsUseTexZShader(modeleffect->GetDefaultMaterialSetting().GetIsUseTexZShader());
	}*/

	void MaterialSetting::Init(MaterialData* materialData) {
		m_isInit = materialData;//���ƂȂ郂�f���G�t�F�N�g���L�^

		//������
		//��ԍŏ��ɍ����e�̃}�e���A���Z�b�e�B���O�������̂���
		SetDefaultVS();
		SetDefaultVSZ();
		SetDefaultPS();
		SetDefaultPSZ();
		SetDefaultAlbedoTexture();
		SetDefaultNormalTexture();
		SetDefaultLightingTexture();

		//�ݒ�̃R�s�[
		SetMatrialName(materialData->GetDefaultMaterialSetting().GetMatrialName());
		SetMaterialParam(materialData->GetDefaultMaterialSetting().GetMaterialParam());

		SetVS(materialData->GetDefaultMaterialSetting().GetVS());
		SetVSZ(materialData->GetDefaultMaterialSetting().GetVSZ());
		SetPS(materialData->GetDefaultMaterialSetting().GetPS());
		SetPSZ(materialData->GetDefaultMaterialSetting().GetPSZ());

		SetAlbedoTexture(materialData->GetDefaultMaterialSetting().GetAlbedoTexture());
		SetNormalTexture(materialData->GetDefaultMaterialSetting().GetNormalTexture());
		SetLightingTexture(materialData->GetDefaultMaterialSetting().GetLightingTexture());
		SetTranslucentTexture(materialData->GetDefaultMaterialSetting().GetTranslucentTexture());

		SetIsMotionBlur(materialData->GetDefaultMaterialSetting().GetIsMotionBlur());
		SetIsUseTexZShader(materialData->GetDefaultMaterialSetting().GetIsUseTexZShader());
	}

	//�V�F�[�_���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultVS() {
		m_pVSShader = m_isInit->GetDefaultVS();
	}
	void MaterialSetting::SetDefaultVSZ() {
		m_pVSZShader = m_isInit->GetDefaultVSZ();
	}
	void MaterialSetting::SetDefaultPS() {
		m_pPSShader = m_isInit->GetDefaultPS();
	}
	void MaterialSetting::SetDefaultPSZ() {
		m_pPSZShader = m_isInit->GetDefaultPSZ();
	}
	//TriPlanarMapping�p�̃V�F�[�_��ݒ�
	void MaterialSetting::SetTriPlanarMappingPS(bool isYOnly) {
		m_pPSShader = m_isInit->GetTriPlanarMappingPS(isYOnly);
	}

	//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultAlbedoTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultAlbedoTexture().textureView.Get();
		SetAlbedoTexture(DT);
	}
	//�m�[�}���}�b�v���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultNormalTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultNormalTexture().textureView.Get();
		SetNormalTexture(DT);
	}
	//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
	void MaterialSetting::SetDefaultLightingTexture() {
		ID3D11ShaderResourceView* DT = m_isInit->GetDefaultLightingTexture().textureView.Get();
		SetLightingTexture(DT);
	}

	//�X�L�����f�����擾
	bool MaterialSetting::GetIsSkining() const{
		return m_isInit->GetIsSkining();
	}

}