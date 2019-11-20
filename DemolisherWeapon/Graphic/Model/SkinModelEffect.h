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
public:
	//�V�F�[�_�[���[�h
	enum enShaderMode {
		enNormalShader,
		enZShader,
	};

	//�V�F�[�_�[���[�h��ݒ�
	static void SetShaderMode(enShaderMode sm) {
		m_s_shadermode = sm;
	}

private:
	static enShaderMode m_s_shadermode ;//�V�F�[�_�[���[�h

protected:
	//�g�p���̃V�F�[�_�[�̃|�C���^
	//SKEShaderPtr m_pVSShader; Shader* m_pVSZShader = nullptr;
	//SKEShaderPtr m_pPSShader; Shader* m_pPSZShader = nullptr;

	//�f�t�H���g�o�[�e�b�N�X�V�F�[�_
	SkinModelEffectShader m_vsDefaultShader;
	Shader m_vsZShader;//Z�l�o�͗p

	//�f�t�H���g�s�N�Z���V�F�[�_
	//bool m_isUseTexZShader = false;
	SkinModelEffectShader m_psDefaultShader;
	Shader m_psZShader[2];//Z�l�o�͗p
	SkinModelEffectShader m_psTriPlanarMapShader;//TriPlanarMapping�p�̃V�F�[�_

	bool isSkining;//�X�L�����f�����H

	//�e�N�X�`��
	ID3D11ShaderResourceView* m_defaultAlbedoTex = nullptr;// , *m_pAlbedoTex = nullptr;
	ID3D11ShaderResourceView* m_defaultNormalTex = nullptr;//, *m_pNormalTex = nullptr;
	ID3D11ShaderResourceView* m_defaultLightingTex = nullptr;//, *m_pLightingTex = nullptr;
	
	MaterialSetting* m_ptrUseMaterialSetting = nullptr;	//�g�p����}�e���A���ݒ�
	MaterialSetting m_defaultMaterialSetting;	//�}�e���A���ݒ�(�f�t�H���g)
	//MaterialParam m_materialParam;			//�}�e���A���p�����[�^
	ID3D11Buffer* m_materialParamCB = nullptr;	//�}�e���A���p�����[�^�p�̒萔�o�b�t�@

	//bool m_enableMotionBlur = true;//���[�V�����u���[�L�����H

public:
	ModelEffect()
	{
		//�}�N�����ƂɃs�N�Z���V�F�[�_���쐬
		m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psTriPlanarMapShader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
		
		//�}�N�����Ƃ�Z�l�`��s�N�Z���V�F�[�_���쐬
		D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
		m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
		m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		
		//�f�t�H���g�̃V�F�[�_��ݒ�
		//m_pPSShader = &m_psDefaultShader;
		//m_pPSZShader = &m_psZShader[0];

		//�}�e���A���ݒ�(m_defaultMaterialSetting)�����������Ă��
		MaterialSettingInit(m_defaultMaterialSetting);

		//�g�p����}�e���A���ݒ���Z�b�g
		SetDefaultMaterialSetting();

		//�}�e���A���p�����[�^�[�̒萔�o�b�t�@
		ShaderUtil::CreateConstantBuffer(sizeof(MaterialParam), &m_materialParamCB);
	}
	virtual ~ModelEffect()
	{
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
	}
	void __cdecl Apply(ID3D11DeviceContext* deviceContext) override;

	void __cdecl GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength) override
	{
		//
		*pShaderByteCode = m_vsDefaultShader.GetShader(SkinModelEffectShader::enALL).GetByteCode();
		*pByteCodeLength = m_vsDefaultShader.GetShader(SkinModelEffectShader::enALL).GetByteCodeSize();
	}

	//�X�L�����f�����ǂ����擾
	bool GetIsSkining()const {
		return isSkining;
	}

	//�}�e���A���ݒ�����̃C���X�^���X���x�[�X�ɏ��������Ă��
	//MaterialSetting& matset�@����������Z�b�e�B���O
	void MaterialSettingInit(MaterialSetting& matset) {
		matset.Init(this);
	}
	//�f�t�H���g�}�e���A���ݒ�̎擾
	MaterialSetting& GetDefaultMaterialSetting() {
		return m_defaultMaterialSetting;
	}

	/// <summary>
	/// �A���x�h�e�N�X�`����ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	/// <param name="defaultTex">�f�t�H���g�e�N�X�`���Ƃ��Đݒ肷�邩?</param>
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex, bool defaultTex = false){
		if (!m_defaultAlbedoTex && defaultTex) {
			//�f�t�H���g�e�N�X�`��
			m_defaultAlbedoTex = tex;
			//m_pAlbedoTex = m_defaultAlbedoTex;
		}
		//�e�N�X�`���ύX
		m_defaultMaterialSetting.SetAlbedoTexture(tex);		
	}
	//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
	void SetDefaultAlbedoTexture() {
		m_defaultMaterialSetting.SetDefaultAlbedoTexture();
	}
	//�f�t�H���g�̃A���x�h�e�N�X�`�����擾
	ID3D11ShaderResourceView* GetDefaultAlbedoTexture() const{
		return m_defaultAlbedoTex;
	}

	/// <summary>
	/// �m�[�}���}�b�v��ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	/// <param name="defaultTex">�f�t�H���g�e�N�X�`���Ƃ��Đݒ肷�邩?</param>
	void SetNormalTexture(ID3D11ShaderResourceView* tex, bool defaultTex = false) {
		if (!m_defaultNormalTex && defaultTex) {
			//�f�t�H���g�e�N�X�`��
			m_defaultNormalTex = tex;
			//m_pNormalTex = m_defaultNormalTex;
		}
		//�e�N�X�`���ύX
		m_defaultMaterialSetting.SetNormalTexture(tex);
	}
	//�m�[�}���}�b�v���f�t�H�ɖ߂�
	void SetDefaultNormalTexture() {
		m_defaultMaterialSetting.SetDefaultNormalTexture();
	}
	//�f�t�H���g�̃m�[�}���}�b�v���擾
	ID3D11ShaderResourceView* GetDefaultNormalTexture() const {
		return m_defaultNormalTex;
	}

	/// <summary>
	/// ���C�e�B���O�p�����[�^�}�b�v��ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	/// <param name="defaultTex">�f�t�H���g�e�N�X�`���Ƃ��Đݒ肷�邩?</param>
	void SetLightingTexture(ID3D11ShaderResourceView* tex, bool defaultTex = false) {
		if (!m_defaultLightingTex && defaultTex) {
			//�f�t�H���g�e�N�X�`��
			m_defaultLightingTex = tex;
			//m_pLightingTex = m_defaultLightingTex;
		}
		//�e�N�X�`���ύX
		m_defaultMaterialSetting.SetLightingTexture(tex);
	}
	//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
	void SetDefaultLightingTexture() {
		m_defaultMaterialSetting.SetDefaultLightingTexture();
	}
	//�f�t�H���g�̃��C�e�B���O�p�����[�^�}�b�v���擾
	ID3D11ShaderResourceView* GetDefaultLightingTexture() const {
		return m_defaultLightingTex;
	}

	/// <summary>
	/// �g�����X���[�Z���g�}�b�v��ݒ�
	/// </summary>
	/// <param name="tex">�ݒ肷��e�N�X�`��</param>
	void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {
		m_defaultMaterialSetting.SetTranslucentTexture(tex);
	}

	//�V�F�[�_��ݒ�
	void SetVS(Shader* vs) {
		m_defaultMaterialSetting.SetVS(vs);
	}
	void SetVSZ(Shader* vs) {
		m_defaultMaterialSetting.SetVSZ(vs);
	}
	void SetPS(Shader* ps) {
		m_defaultMaterialSetting.SetPS(ps);
	}
	//�V�F�[�_���f�t�H�ɖ߂�
	void SetDefaultVS() {
		m_defaultMaterialSetting.SetDefaultVS();
	}
	void SetDefaultVSZ() {
		m_defaultMaterialSetting.SetDefaultVSZ();
	}
	void SetDefaultPS() {
		m_defaultMaterialSetting.SetDefaultPS();
	}
	//�f�t�H���g�̃V�F�[�_���擾
	SkinModelEffectShader* GetDefaultVS() {
		return &m_vsDefaultShader;
	}
	Shader* GetDefaultVSZ() {
		return &m_vsZShader;
	}
	SkinModelEffectShader* GetDefaultPS() {
		return &m_psDefaultShader;
	}
	Shader* GetDefaultPSZ() {
		return &m_psZShader[0];
	}
	//TriPlanarMapping�p�̃V�F�[�_���擾
	SkinModelEffectShader* GetTriPlanarMappingPS() {
		return &m_psTriPlanarMapShader;
	}

	//���O��ݒ�
	void SetMatrialName(const wchar_t* matName)
	{
		m_defaultMaterialSetting.SetMatrialName(matName);
	}
	//���O���擾
	const wchar_t* GetMatrialName()const {
		return m_defaultMaterialSetting.GetMatrialName();
	}
	//���O�̈�v�𔻒�
	bool EqualMaterialName(const wchar_t* name) const
	{
		return m_defaultMaterialSetting.EqualMaterialName(name);
	}

	//���C�e�B���O���邩��ݒ�
	void SetLightingEnable(bool enable) {
		m_defaultMaterialSetting.SetLightingEnable(enable);
	}
	//���Ȕ����F(�G�~�b�V�u)��ݒ�
	void SetEmissive(float emissive) {
		m_defaultMaterialSetting.SetEmissive(emissive);
	}
	//�A���x�h�ɂ�����X�P�[����ݒ�
	void SetAlbedoScale(const CVector4& scale) {
		m_defaultMaterialSetting.SetAlbedoScale(scale);
	}


	//�����艺�A�����p

	//�g���}�e���A���ݒ��K��
	void SetUseMaterialSetting(MaterialSetting& matset) {
		m_ptrUseMaterialSetting = &matset;
		/*m_materialParam = matset.GetMaterialParam();
		
		m_pVSShader = matset.GetVS();
		m_pVSZShader = matset.GetVSZ(); 
		m_pPSShader = matset.GetPS();
		m_pPSZShader = matset.GetPSZ();

		m_pAlbedoTex = matset.GetAlbedoTexture();
		m_pNormalTex = matset.GetNormalTexture();
		m_pLightingTex = matset.GetLightingTexture();

		m_enableMotionBlur = matset.GetIsMotionBlur();
		m_isUseTexZShader = matset.GetIsUseTexZShader();*/		
	}
	void SetDefaultMaterialSetting() {
		SetUseMaterialSetting(m_defaultMaterialSetting);
	}
};
/*!
*@brief
*  �X�L���Ȃ����f���G�t�F�N�g�B
*/
class NonSkinModelEffect : public ModelEffect {
public:
	NonSkinModelEffect()
	{
		//�}�N�����Ƃɒ��_�V�F�[�_���쐬
		m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
		
		//Z�l�`��V�F�[�_���쐬
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);

		//�f�t�H���g�̃V�F�[�_��ݒ�
		//m_pVSShader = &m_vsDefaultShader;
		//m_pVSZShader = &m_vsZShader;

		//�X�L�����f������Ȃ�
		isSkining = false;
	}
};
/*!
*@brief
*  �X�L�����f���G�t�F�N�g�B
*/
class SkinModelEffect : public ModelEffect {
public:
	SkinModelEffect()
	{
		//�}�N�����Ƃɒ��_�V�F�[�_���쐬
		m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
		
		//Z�l�`��V�F�[�_���쐬
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		//�f�t�H���g�̃V�F�[�_��ݒ�
		//m_pVSShader = &m_vsDefaultShader;
		//m_pVSZShader = &m_vsZShader;

		//�X�L�����f���ł���
		isSkining = true;
	}
};

/*!
*@brief
*  �G�t�F�N�g�t�@�N�g���B
*/
class SkinModelEffectFactory : public DirectX::EffectFactory {
public:
	SkinModelEffectFactory(ID3D11Device* device) :
		EffectFactory(device) {}
	std::shared_ptr<DirectX::IEffect> __cdecl CreateEffect(const EffectInfo& info, ID3D11DeviceContext* deviceContext)override
	{
		//���f���G�t�F�N�g�쐬
		std::shared_ptr<ModelEffect> effect;
		if (info.enableSkinning) {
			//�X�L�j���O����B
			effect = std::make_shared<SkinModelEffect>();
		}
		else {
			//�X�L�j���O�Ȃ��B
			effect = std::make_shared<NonSkinModelEffect>();
		}

		//���O�ݒ�
		effect->GetDefaultMaterialSetting().SetMatrialName(info.name);

		//�A���x�h
		if (info.diffuseTexture && *info.diffuseTexture)
		{
			//�e�N�X�`���Őݒ�
			ID3D11ShaderResourceView* texSRV;
			DirectX::EffectFactory::CreateTexture(info.diffuseTexture, deviceContext, &texSRV);
			effect->SetAlbedoTexture(texSRV,true);
		}
		else {
			//�f�B�t���[�Y�J���[(���l)�Őݒ�
			effect->SetAlbedoScale({ info.diffuseColor.x,info.diffuseColor.y,info.diffuseColor.z,1.0f });
		}

		//�m�[�}��
		if (info.normalTexture && *info.normalTexture)
		{
			//�e�N�X�`���Őݒ�
			ID3D11ShaderResourceView* texSRV;
			DirectX::EffectFactory::CreateTexture(info.normalTexture, deviceContext, &texSRV);
			effect->SetNormalTexture(texSRV, true);
		}

		//���C�e�B���O�p�����[�^
		if (info.specularTexture && *info.specularTexture)
		{
			//�e�N�X�`���Őݒ�
			ID3D11ShaderResourceView* texSRV;
			DirectX::EffectFactory::CreateTexture(info.specularTexture, deviceContext, &texSRV);
			effect->SetLightingTexture(texSRV, true);
		}

		return effect;
	}

	void __cdecl CreateTexture(const wchar_t* name, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** textureView) override
	{
		return DirectX::EffectFactory::CreateTexture(name, deviceContext, textureView);
	}
};

}