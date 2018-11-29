#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"

namespace DemolisherWeapon {

/*!
*@brief	���f���G�t�F�N�g�B
*/
class ModelEffect : public DirectX::IEffect {
public:
	enum enShaderMode {
		enNormalShader,
		enZShader,
	};

	//�g�p����V�F�[�_�[���[�h��ݒ�
	static void SetShaderMode(enShaderMode sm) {
		m_s_shadermode = sm;
	}

private:
	static enShaderMode m_s_shadermode ;

protected:
	std::wstring m_materialName;	//!<�}�e���A�����B
	Shader* m_pVSShader = nullptr;
	Shader* m_pPSShader = nullptr;
	Shader m_vsShader, m_vsZShader;
	Shader m_psShader, m_psZShader;
	bool isSkining;
	ID3D11ShaderResourceView* m_albedoTex = nullptr, *m_pAlbedoTex = nullptr;

	//�萔�o�b�t�@�@[model.fx:MaterialCb]
	//�}�e���A���p�����[�^�[
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();	//�A���x�h�ɂ�����X�P�[��
		CVector3 emissive;						//�G�~�b�V�u(���Ȕ���)
		int isLighting = 1;						//���C�e�B���O���邩
	};
	MaterialParam m_materialParam;				//�}�e���A���p�����[�^
	ID3D11Buffer* m_materialParamCB = nullptr;	//�}�e���A���p�����[�^�p�̒萔�o�b�t�@

public:
	ModelEffect()
	{
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psZShader.Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);

		m_pPSShader = &m_psShader;

		//�}�e���A���p�����[�^�[�̒萔�o�b�t�@
		ShaderUtil::CreateConstantBuffer(sizeof(MaterialParam), &m_materialParamCB);
	}
	virtual ~ModelEffect()
	{
		if (m_albedoTex) {
			m_albedoTex->Release();
		}
		if (m_materialParamCB)m_materialParamCB->Release();
	}
	void __cdecl Apply(ID3D11DeviceContext* deviceContext) override;

	void __cdecl GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength) override
	{
		*pShaderByteCode = m_vsShader.GetByteCode();
		*pByteCodeLength = m_vsShader.GetByteCodeSize();
	}

	//�A���x�h�e�N�X�`����ݒ�
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex)
	{
		if (!m_albedoTex) {
			//�f�t�H���g�e�N�X�`��
			m_albedoTex = tex;
			m_pAlbedoTex = m_albedoTex;
		}
		else {
			//�e�N�X�`���ύX
			m_pAlbedoTex = tex;
			m_pAlbedoTex->AddRef();
		}
	}
	//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
	void SetDefaultAlbedoTexture() {
		if (m_pAlbedoTex == m_albedoTex) { return; }//���Ƀf�t�H���g�e�N�X�`��

		m_pAlbedoTex->Release();
		m_pAlbedoTex = m_albedoTex;
	}

	//���O���擾
	void SetMatrialName(const wchar_t* matName)
	{
		m_materialName = matName;
	}	
	//���O�̈�v�𔻒�
	bool EqualMaterialName(const wchar_t* name) const
	{
		return wcscmp(name, m_materialName.c_str()) == 0;
	}

	//�V�F�[�_��ݒ�
	void SetPS(Shader* ps) {
		m_pPSShader = ps;
	}
	//�V�F�[�_���f�t�H�ɖ߂�
	void SetDefaultPS() {
		m_pPSShader = &m_psShader;
	}

	//���C�e�B���O���邩��ݒ�
	void SetLightingEnable(bool enable) {
		m_materialParam.isLighting = enable ? 1 : 0;
	}

	//���Ȕ����F(�G�~�b�V�u)��ݒ�
	void SetEmissive(const CVector3& emissive) {
		m_materialParam.emissive = emissive;
	}

	//�A���x�h�ɂ�����X�P�[����ݒ�
	void SetAlbedoScale(const CVector4& scale) {
		m_materialParam.albedoScale = scale;
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
		m_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);
		
		m_pVSShader = &m_vsShader;
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
		wchar_t hoge[256];
		GetCurrentDirectoryW(256, hoge);
		m_vsShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		m_pVSShader = &m_vsShader;
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
		std::shared_ptr<ModelEffect> effect;
		if (info.enableSkinning) {
			//�X�L�j���O����B
			effect = std::make_shared<SkinModelEffect>();
		}
		else {
			//�X�L�j���O�Ȃ��B
			effect = std::make_shared<NonSkinModelEffect>();
		}
		effect->SetMatrialName(info.name);
		if (info.diffuseTexture && *info.diffuseTexture)
		{
			ID3D11ShaderResourceView* texSRV;
			DirectX::EffectFactory::CreateTexture(info.diffuseTexture, deviceContext, &texSRV);
			effect->SetAlbedoTexture(texSRV);
		}
		return effect;
	}

	void __cdecl CreateTexture(const wchar_t* name, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView** textureView) override
	{
		return DirectX::EffectFactory::CreateTexture(name, deviceContext, textureView);
	}
};

}