#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"
#include "Graphic/Shader/ShaderResources.h"

#include "MaterialSetting.h"

namespace DemolisherWeapon {

#include "Preset/Shader/ShadowMapHeader.h"

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
	Shader* m_pVSShader = nullptr, *m_pVSZShader = nullptr;
	Shader* m_pPSShader = nullptr;

	enum ShaderTypeMask {
		enOFF = 0b00,
		enMotionBlur = 0b01,
		enNormalMap = 0b10,
		enALL = 0b11,
		enNum,
	};
	D3D_SHADER_MACRO macros[enALL] = {
			"MOTIONBLUR", "0",
			"NORMAL_MAP", "0",
			NULL, NULL
	};

	Shader m_vsDefaultShader[ShaderTypeMask::enNum], m_vsZShader;
	//int m_clacOldPosOffset = 0;
	//ID3D11ClassInstance* m_cCalcOldPos = nullptr, *m_cNoCalcOldPos = nullptr;

	bool m_isUseTexZShader = false;
	Shader m_psDefaultShader[ShaderTypeMask::enNum], m_psZShader[2];
	//int m_clacVelocityOffset = 0;
	//ID3D11ClassInstance* m_cCalcVelocity = nullptr, *m_cNoCalcVelocity = nullptr;	

	bool isSkining;
	ID3D11ShaderResourceView* m_albedoTex = nullptr, *m_pAlbedoTex = nullptr;
	ID3D11ShaderResourceView* m_pNormalTex = nullptr;
	
	MaterialSetting m_defaultMaterialSetting;	//�}�e���A���ݒ�
	MaterialParam m_materialParam;				//�}�e���A���p�����[�^
	ID3D11Buffer* m_materialParamCB = nullptr;	//�}�e���A���p�����[�^�p�̒萔�o�b�t�@

	bool m_enableMotionBlur = true;
public:
	ModelEffect()
	{
		char macroName[32];
		for (int i = 0; i < ShaderTypeMask::enNum; i++) {
			sprintf_s(macroName, "DefaultModelShader:%d", i);

			for (int mask = ShaderTypeMask::enOFF+1; mask < ShaderTypeMask::enALL; mask++) {
				if (i & mask) { macros[mask - 1].Definition = "1"; }else{ macros[mask - 1].Definition = "0"; }
			}

			m_psDefaultShader[i].Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, macroName, macros);
		}

		D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
		m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
		m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		
		LoadClassInstancePS();

		m_pPSShader = &m_psDefaultShader[enALL];

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

		/*if (m_cCalcOldPos) {
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
		}*/
	}
	void __cdecl Apply(ID3D11DeviceContext* deviceContext) override;

	void __cdecl GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength) override
	{
		*pShaderByteCode = m_vsDefaultShader[enALL].GetByteCode();
		*pByteCodeLength = m_vsDefaultShader[enALL].GetByteCodeSize();
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

	//�m�[�}���}�b�v��ݒ�
	void SetNormalTexture(ID3D11ShaderResourceView* tex) {
		m_defaultMaterialSetting.SetNormalTexture(tex);
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
	Shader* GetDefaultVS() {
		return &m_vsDefaultShader[enALL];
	}
	Shader* GetDefaultVSZ() {
		return &m_vsZShader;
	}
	Shader* GetDefaultPS() {
		return &m_psDefaultShader[enALL];
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
		m_materialParam = matset.GetMaterialParam();
		m_pVSShader = matset.GetVS();
		m_pVSZShader = matset.GetVSZ(); 
		m_pPSShader = matset.GetPS();
		m_pAlbedoTex = matset.GetAlbedoTexture();
		m_pNormalTex = matset.GetNormalTexture();
		m_enableMotionBlur = matset.GetIsMotionBlur();
		m_isUseTexZShader = matset.GetIsUseTexZShader();

		/*if (m_pVSShader == &m_vsDefaultShader) {
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
		}*/
	}
	void SetDefaultMaterialSetting() {
		SetUseMaterialSetting(m_defaultMaterialSetting);
	}
	
protected:
	//���I�����N
	void LoadClassInstanceVS(){
		/*
		//�I�t�Z�b�g�擾
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(m_vsDefaultShader.GetByteCode(), m_vsDefaultShader.GetByteCodeSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		ID3D11ShaderReflectionVariable* pAmbientLightingVar = pReflector->GetVariableByName("g_calcOldPos");
		m_clacOldPosOffset = pAmbientLightingVar->GetInterfaceSlot(0);

		pReflector->Release();

		//�C���X�^���X�擾
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cCalcOldPos", 0, 0, 0, 0, &m_cCalcOldPos);
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cNotCalcOldPos", 0, 0, 0, 0, &m_cNoCalcOldPos);
		*/
	}
	void LoadClassInstancePS() {
		/*
		//�I�t�Z�b�g�擾
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(m_psDefaultShader.GetByteCode(), m_psDefaultShader.GetByteCodeSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		ID3D11ShaderReflectionVariable* pAmbientLightingVar = pReflector->GetVariableByName("g_calcVelocity");
		m_clacVelocityOffset = pAmbientLightingVar->GetInterfaceSlot(0);

		pReflector->Release();

		//�C���X�^���X�擾
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cCalcVelocity", 0, 0, 0, 0, &m_cCalcVelocity);
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cNotCalcVelocity", 0, 0, 0, 0, &m_cNoCalcVelocity);
		*/
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
		char macroName[32];
		for (int i = 0; i < ShaderTypeMask::enNum; i++) {
			sprintf_s(macroName, "DefaultModelShader:%d", i);

			for (int mask = ShaderTypeMask::enOFF + 1; mask < ShaderTypeMask::enALL; mask++) {
				if (i & mask) { macros[mask - 1].Definition = "1"; }else { macros[mask - 1].Definition = "0"; }
			}

			m_vsDefaultShader[i].Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, macroName, macros);
		}
		
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();
		
		m_pVSShader = &m_vsDefaultShader[enALL];
		m_pVSZShader = &m_vsZShader;
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
		char macroName[32];		
		for (int i = 0; i < ShaderTypeMask::enNum; i++) {
			sprintf_s(macroName, "DefaultModelShader:%d", i);

			for (int mask = ShaderTypeMask::enOFF + 1; mask < ShaderTypeMask::enALL; mask++) {
				if (i & mask) { macros[mask - 1].Definition = "1"; }else { macros[mask - 1].Definition = "0"; }
			}

			m_vsDefaultShader[i].Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, macroName, macros);
		}
		
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();

		m_pVSShader = &m_vsDefaultShader[enALL];
		m_pVSZShader = &m_vsZShader;
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