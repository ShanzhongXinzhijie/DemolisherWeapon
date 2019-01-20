#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"
#include "Graphic/Shader/ShaderResources.h"

#include "MaterialSetting.h"

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
	Shader* m_pVSShader = nullptr;
	Shader* m_pPSShader = nullptr;

	Shader m_vsDefaultShader, m_vsZShader;
	int m_clacOldPosOffset = 0;
	ID3D11ClassInstance* m_cCalcOldPos = nullptr, *m_cNoCalcOldPos = nullptr;

	Shader m_psDefaultShader, m_psZShader;
	int m_clacVelocityOffset = 0;
	ID3D11ClassInstance* m_cCalcVelocity = nullptr, *m_cNoCalcVelocity = nullptr;

	bool isSkining;
	ID3D11ShaderResourceView* m_albedoTex = nullptr, *m_pAlbedoTex = nullptr;
	
	MaterialSetting m_defaultMaterialSetting;	//�}�e���A���ݒ�
	MaterialParam m_materialParam;				//�}�e���A���p�����[�^
	ID3D11Buffer* m_materialParamCB = nullptr;	//�}�e���A���p�����[�^�p�̒萔�o�b�t�@

public:
	ModelEffect()
	{
		m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psZShader.Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);

		LoadClassInstancePS();

		m_pPSShader = &m_psDefaultShader;

		MaterialSettingInit(m_defaultMaterialSetting);

		//�}�e���A���p�����[�^�[�̒萔�o�b�t�@
		ShaderUtil::CreateConstantBuffer(sizeof(MaterialParam), &m_materialParamCB);
	}
	virtual ~ModelEffect()
	{
		if (m_albedoTex) {
			m_albedoTex->Release();
		}
		if (m_materialParamCB) {
			m_materialParamCB->Release();
		}

		if (m_cCalcOldPos) {
			m_cCalcOldPos->Release();
		}
		if (m_cNoCalcOldPos) {
			m_cNoCalcOldPos->Release();
		}
		if (m_cCalcVelocity) {
			m_cCalcVelocity->Release();
		}
		if (m_cNoCalcVelocity) {
			m_cNoCalcVelocity->Release();
		}
	}
	void __cdecl Apply(ID3D11DeviceContext* deviceContext) override;

	void __cdecl GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength) override
	{
		*pShaderByteCode = m_vsDefaultShader.GetByteCode();
		*pByteCodeLength = m_vsDefaultShader.GetByteCodeSize();
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

	//�A���x�h�e�N�X�`����ݒ�
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex){
		if (!m_albedoTex) {
			//�f�t�H���g�e�N�X�`��
			m_albedoTex = tex;
			m_pAlbedoTex = m_albedoTex;
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
		return m_albedoTex;
	}

	//�V�F�[�_��ݒ�
	void SetPS(Shader* ps) {
		m_defaultMaterialSetting.SetPS(ps);
	}
	//�V�F�[�_���f�t�H�ɖ߂�
	void SetDefaultPS() {
		m_defaultMaterialSetting.SetDefaultPS();
	}
	//�f�t�H���g�̃V�F�[�_���擾
	Shader* GetDefaultPS() {
		return &m_psDefaultShader;
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
	void SetEmissive(const CVector3& emissive) {
		m_defaultMaterialSetting.SetEmissive(emissive);
	}
	//�A���x�h�ɂ�����X�P�[����ݒ�
	void SetAlbedoScale(const CVector4& scale) {
		m_defaultMaterialSetting.SetAlbedoScale(scale);
	}


	//�����艺�A�����p

	//�g���}�e���A���ݒ��K��
	void SetUseMaterialSetting(MaterialSetting& matset) {
		m_materialParam = matset.GetMaterialParam();
		m_pPSShader = matset.GetPS();
		m_pAlbedoTex = matset.GetAlbedoTexture();

		if (m_pVSShader == &m_vsDefaultShader) {
			if (matset.GetIsMotionBlur()) {
				ID3D11ClassInstance** array = m_vsDefaultShader.GetClassInstanceArray();
				array[m_clacOldPosOffset] = m_cCalcOldPos;
			}
			else {
				ID3D11ClassInstance** array = m_vsDefaultShader.GetClassInstanceArray();
				array[m_clacOldPosOffset] = m_cNoCalcOldPos;
			}
		}
		if (m_pPSShader == &m_psDefaultShader) {
			if (matset.GetIsMotionBlur()) {
				ID3D11ClassInstance** array = m_psDefaultShader.GetClassInstanceArray();
				array[m_clacVelocityOffset] = m_cCalcVelocity;
			}
			else {
				ID3D11ClassInstance** array = m_psDefaultShader.GetClassInstanceArray();
				array[m_clacVelocityOffset] = m_cNoCalcVelocity;
			}
		}
	}
	void SetDefaultMaterialSetting() {
		SetUseMaterialSetting(m_defaultMaterialSetting);
	}
	
protected:
	//���I�����N
	void LoadClassInstanceVS(){
		//�I�t�Z�b�g�擾
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(m_vsDefaultShader.GetByteCode(), m_vsDefaultShader.GetByteCodeSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		ID3D11ShaderReflectionVariable* pAmbientLightingVar = pReflector->GetVariableByName("g_calcOldPos");
		m_clacOldPosOffset = pAmbientLightingVar->GetInterfaceSlot(0);

		pReflector->Release();

		//�C���X�^���X�擾
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cCalcOldPos", 0, 0, 0, 0, &m_cCalcOldPos);
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cNotCalcOldPos", 0, 0, 0, 0, &m_cNoCalcOldPos);
	}
	void LoadClassInstancePS() {
		//�I�t�Z�b�g�擾
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(m_psDefaultShader.GetByteCode(), m_psDefaultShader.GetByteCodeSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		ID3D11ShaderReflectionVariable* pAmbientLightingVar = pReflector->GetVariableByName("g_calcVelocity");
		m_clacVelocityOffset = pAmbientLightingVar->GetInterfaceSlot(0);

		pReflector->Release();

		//�C���X�^���X�擾
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cCalcVelocity", 0, 0, 0, 0, &m_cCalcVelocity);
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cNotCalcVelocity", 0, 0, 0, 0, &m_cNoCalcVelocity);
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
		m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();
		
		m_pVSShader = &m_vsDefaultShader;
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
		m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();

		m_pVSShader = &m_vsDefaultShader;
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

		//�e�N�X�`���ݒ�
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