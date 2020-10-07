#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"
#include "Graphic/Shader/ShaderResources.h"

#include "SkinModelEffectShader.h"

#include "MaterialSetting.h"

namespace DemolisherWeapon {

#include "Preset/Shader/ShadowMapHeader.h"

/*!
*@brief	���f���G�t�F�N�g�B
*/
class ModelEffect : public DirectX::IEffect {
protected:
	/*
	//�f�t�H���g�o�[�e�b�N�X�V�F�[�_
	SkinModelEffectShader m_vsDefaultShader;
	Shader m_vsZShader;//Z�l�o�͗p

	//�f�t�H���g�s�N�Z���V�F�[�_
	SkinModelEffectShader m_psDefaultShader;
	Shader m_psZShader[2];//Z�l�o�͗p
	SkinModelEffectShader m_psTriPlanarMapShader, m_psTriPlanarMapShaderYOnly;//TriPlanarMapping�p�̃V�F�[�_

	bool isSkining;//�X�L�����f�����H

	//�e�N�X�`��
	ID3D11ShaderResourceView* m_defaultAlbedoTex = nullptr;
	ID3D11ShaderResourceView* m_defaultNormalTex = nullptr;
	ID3D11ShaderResourceView* m_defaultLightingTex = nullptr;
	
	MaterialSetting* m_ptrUseMaterialSetting = nullptr;	//�g�p����}�e���A���ݒ�
	MaterialSetting m_defaultMaterialSetting;	//�}�e���A���ݒ�(�f�t�H���g)
	ID3D11Buffer* m_materialParamCB = nullptr;	//�}�e���A���p�����[�^�p�̒萔�o�b�t�@
	*/
	MaterialData m_matData;

public:
	ModelEffect()
	{
		/*
		//�}�N�����ƂɃs�N�Z���V�F�[�_���쐬
		m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psTriPlanarMapShader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
		D3D_SHADER_MACRO macrosYOnly[] = { "Y_ONLY", "1", NULL, NULL };
		m_psTriPlanarMapShaderYOnly.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS, "Y_ONLY", macrosYOnly);

		//�}�N�����Ƃ�Z�l�`��s�N�Z���V�F�[�_���쐬
		D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
		m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
		m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		
		//�}�e���A���ݒ�(m_defaultMaterialSetting)�����������Ă��
		MaterialSettingInit(m_defaultMaterialSetting);

		//�g�p����}�e���A���ݒ���Z�b�g
		SetDefaultMaterialSetting();

		//�}�e���A���p�����[�^�[�̒萔�o�b�t�@
		ShaderUtil::CreateConstantBuffer(sizeof(MaterialParam), &m_materialParamCB);
		*/
	}
	virtual ~ModelEffect()
	{
		/*
		if (m_defaultAlbedoTex) {
			m_defaultAlbedoTex->Release();
		}
		if (m_defaultNormalTex) {
			m_defaultNormalTex->Release();
		}
		if (m_defaultLightingTex) {
			m_defaultLightingTex->Release();
		}
		if (m_materialParamCB) {
			m_materialParamCB->Release();
		}
		*/
	}

#ifndef DW_DX12_TEMPORARY
	void __cdecl Apply(ID3D11DeviceContext* deviceContext) override;

	void __cdecl GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength) override
	{
		//
		*pShaderByteCode = m_matData.m_vsDefaultShader.GetShader(SkinModelEffectShader::enALL).GetByteCode();
		*pByteCodeLength = m_matData.m_vsDefaultShader.GetShader(SkinModelEffectShader::enALL).GetByteCodeSize();
	}
#endif

	//�X�L�����f�����ǂ����擾
	bool GetIsSkining()const {
		return m_matData.m_isSkining;
	}

	//�}�e���A���ݒ�����̃C���X�^���X���x�[�X�ɏ��������Ă��
	//MaterialSetting& matset�@����������Z�b�e�B���O
	void MaterialSettingInit(MaterialSetting& matset) {
		matset.Init(&m_matData);
	}
	//�f�t�H���g�}�e���A���ݒ�̎擾
	MaterialSetting& GetDefaultMaterialSetting() {
		return m_matData.m_defaultMaterialSetting;
	}

	/// <summary>
	/// �A���x�h�e�N�X�`����ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex){
		//�e�N�X�`���ύX
		m_matData.m_defaultMaterialSetting.SetAlbedoTexture(tex);
	}
	//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
	void SetDefaultAlbedoTexture() {
		m_matData.m_defaultMaterialSetting.SetDefaultAlbedoTexture();
	}
	//�f�t�H���g�̃A���x�h�e�N�X�`�����擾
	ID3D11ShaderResourceView* GetDefaultAlbedoTexture() const {
		return m_matData.m_defaultMaterialSetting.GetAlbedoTexture();
	}

	/// <summary>
	/// �m�[�}���}�b�v��ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	void SetNormalTexture(ID3D11ShaderResourceView* tex) {
		//�e�N�X�`���ύX
		m_matData.m_defaultMaterialSetting.SetNormalTexture(tex);
	}
	//�m�[�}���}�b�v���f�t�H�ɖ߂�
	void SetDefaultNormalTexture() {
		m_matData.m_defaultMaterialSetting.SetDefaultNormalTexture();
	}
	//�f�t�H���g�̃m�[�}���}�b�v���擾
	ID3D11ShaderResourceView* GetDefaultNormalTexture() const {
		return m_matData.m_defaultMaterialSetting.GetNormalTexture();
	}

	/// <summary>
	/// ���C�e�B���O�p�����[�^�}�b�v��ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	void SetLightingTexture(ID3D11ShaderResourceView* tex) {
		//�e�N�X�`���ύX
		m_matData.m_defaultMaterialSetting.SetLightingTexture(tex);
	}
	//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
	void SetDefaultLightingTexture() {
		m_matData.m_defaultMaterialSetting.SetDefaultLightingTexture();
	}
	//�f�t�H���g�̃��C�e�B���O�p�����[�^�}�b�v���擾
	ID3D11ShaderResourceView* GetDefaultLightingTexture() const {
		return m_matData.m_defaultMaterialSetting.GetLightingTexture();
	}

	/// <summary>
	/// �g�����X���[�Z���g�}�b�v��ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {
		m_matData.m_defaultMaterialSetting.SetTranslucentTexture(tex);
	}

	//�V�F�[�_��ݒ�
	void SetVS(Shader* vs) {
		m_matData.m_defaultMaterialSetting.SetVS(vs);
	}
	void SetVSZ(Shader* vs) {
		m_matData.m_defaultMaterialSetting.SetVSZ(vs);
	}
	void SetPS(Shader* ps) {
		m_matData.m_defaultMaterialSetting.SetPS(ps);
	}
	//�V�F�[�_���f�t�H�ɖ߂�
	void SetDefaultVS() {
		m_matData.m_defaultMaterialSetting.SetDefaultVS();
	}
	void SetDefaultVSZ() {
		m_matData.m_defaultMaterialSetting.SetDefaultVSZ();
	}
	void SetDefaultPS() {
		m_matData.m_defaultMaterialSetting.SetDefaultPS();
	}
	//�f�t�H���g�̃V�F�[�_���擾
	SkinModelEffectShader* GetDefaultVS() {
		return &m_matData.m_vsDefaultShader;
	}
	Shader* GetDefaultVSZ() {
		return &m_matData.m_vsZShader;
	}
	SkinModelEffectShader* GetDefaultPS() {
		return &m_matData.m_psDefaultShader;
	}
	Shader* GetDefaultPSZ() {
		return &m_matData.m_psZShader[0];
	}
	//TriPlanarMapping�p�̃V�F�[�_���擾
	SkinModelEffectShader* GetTriPlanarMappingPS(bool isYOnly) {
		return isYOnly ? &m_matData.m_psTriPlanarMapShaderYOnly : &m_matData.m_psTriPlanarMapShader;
	}

	//���O��ݒ�
	void SetMatrialName(const wchar_t* matName)
	{
		m_matData.m_defaultMaterialSetting.SetMatrialName(matName);
	}
	//���O���擾
	const wchar_t* GetMatrialName()const {
		return m_matData.m_defaultMaterialSetting.GetMatrialName();
	}
	//���O�̈�v�𔻒�
	bool EqualMaterialName(const wchar_t* name) const
	{
		return m_matData.m_defaultMaterialSetting.EqualMaterialName(name);
	}

	//���C�e�B���O���邩��ݒ�
	void SetLightingEnable(bool enable) {
		m_matData.m_defaultMaterialSetting.SetLightingEnable(enable);
	}
	//���Ȕ����F(�G�~�b�V�u)��ݒ�
	void SetEmissive(float emissive) {
		m_matData.m_defaultMaterialSetting.SetEmissive(emissive);
	}
	//�A���x�h�ɂ�����X�P�[����ݒ�
	void SetAlbedoScale(const CVector4& scale) {
		m_matData.m_defaultMaterialSetting.SetAlbedoScale(scale);
	}


	//�����艺�A�����p

	//�g���}�e���A���ݒ��K��
	void SetUseMaterialSetting(MaterialSetting& matset) {
		m_matData.m_ptrUseMaterialSetting = &matset;
	}
	//���f���f�[�^�f�t�H���g�̃}�e���A���ݒ���g�p
	void SetDefaultMaterialSetting() {
		SetUseMaterialSetting(m_matData.m_defaultMaterialSetting);
	}
	/// <summary>
	/// �}�e���A���f�[�^���擾
	/// </summary>
	MaterialData& GetMatrialData() {
		return m_matData;
	}
};

/*!
*@brief
*  �G�t�F�N�g�t�@�N�g���B
*/
#ifndef DW_DX12_TEMPORARY
class SkinModelEffectFactory : public DirectX::EffectFactory {
public:
	SkinModelEffectFactory(ID3D11Device* device) : EffectFactory(device) {}

	std::shared_ptr<DirectX::IEffect> __cdecl CreateEffect(const EffectInfo& info, ID3D11DeviceContext* deviceContext)override
	{
		//���f���G�t�F�N�g�쐬
		std::shared_ptr<ModelEffect> effect = std::make_shared<ModelEffect>();
		
		wchar_t fullName[MAX_PATH] = {};
		
		//�A���x�h
		if (info.diffuseTexture && *info.diffuseTexture)
		{
			//�e�N�X�`���Őݒ�
			wcscpy_s(fullName, GetDirectory());
			wcscat_s(fullName, info.diffuseTexture);
			effect->GetMatrialData().InitAlbedoTexture(fullName);
		}
		else {
			//�f�B�t���[�Y�J���[(���l)�Őݒ�
			effect->GetMatrialData().InitAlbedoColor({ info.diffuseColor.x,info.diffuseColor.y,info.diffuseColor.z });
		}

		//�m�[�}��
		if (info.normalTexture && *info.normalTexture)
		{
			//�e�N�X�`���Őݒ�
			wcscpy_s(fullName, GetDirectory());
			wcscat_s(fullName, info.normalTexture);
			effect->GetMatrialData().InitNormalTexture(fullName);
		}

		//���C�e�B���O�p�����[�^
		if (info.specularTexture && *info.specularTexture)
		{
			//�e�N�X�`���Őݒ�
			wcscpy_s(fullName, GetDirectory());
			wcscat_s(fullName, info.specularTexture);
			effect->GetMatrialData().InitLightingTexture(fullName);
		}

		//������
		effect->GetMatrialData().Init(info.enableSkinning, info.name);

		return effect;
	}

	void __cdecl CreateTexture(const wchar_t* name, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** textureView) override
	{
		return DirectX::EffectFactory::CreateTexture(name, deviceContext, textureView);
	}
};
#endif

}