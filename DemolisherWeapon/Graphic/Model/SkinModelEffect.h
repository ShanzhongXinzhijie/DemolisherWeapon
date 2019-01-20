#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"
#include "Graphic/Shader/ShaderResources.h"

#include "MaterialSetting.h"

namespace DemolisherWeapon {

/*!
*@brief	モデルエフェクト。
*/
class ModelEffect : public DirectX::IEffect {
public:
	enum enShaderMode {
		enNormalShader,
		enZShader,
	};

	//使用するシェーダーモードを設定
	static void SetShaderMode(enShaderMode sm) {
		m_s_shadermode = sm;
	}

private:
	static enShaderMode m_s_shadermode ;

protected:
	Shader* m_pVSShader = nullptr;
	Shader* m_pPSShader = nullptr;

	Shader m_vsDefaultShader[2], m_vsZShader;
	//int m_clacOldPosOffset = 0;
	//ID3D11ClassInstance* m_cCalcOldPos = nullptr, *m_cNoCalcOldPos = nullptr;

	Shader m_psDefaultShader[2], m_psZShader;
	//int m_clacVelocityOffset = 0;
	//ID3D11ClassInstance* m_cCalcVelocity = nullptr, *m_cNoCalcVelocity = nullptr;

	enum {
		enALL,
		enNoMotionBlur,
	};

	bool isSkining;
	ID3D11ShaderResourceView* m_albedoTex = nullptr, *m_pAlbedoTex = nullptr;
	
	MaterialSetting m_defaultMaterialSetting;	//マテリアル設定
	MaterialParam m_materialParam;				//マテリアルパラメータ
	ID3D11Buffer* m_materialParamCB = nullptr;	//マテリアルパラメータ用の定数バッファ

public:
	ModelEffect()
	{
		D3D_SHADER_MACRO macros[] = { "MOTIONBLUR", "1", NULL, NULL };
		m_psDefaultShader[enALL].Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "ALL", macros);
		m_psDefaultShader[enNoMotionBlur].Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psZShader.Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);

		LoadClassInstancePS();

		m_pPSShader = &m_psDefaultShader[enALL];

		MaterialSettingInit(m_defaultMaterialSetting);

		//マテリアルパラメーターの定数バッファ
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

	//マテリアル設定をこのインスタンスをベースに初期化してやる
	//MaterialSetting& matset　初期化するセッティング
	void MaterialSettingInit(MaterialSetting& matset) {
		matset.Init(this);
	}
	//デフォルトマテリアル設定の取得
	MaterialSetting& GetDefaultMaterialSetting() {
		return m_defaultMaterialSetting;
	}

	//アルベドテクスチャを設定
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex){
		if (!m_albedoTex) {
			//デフォルトテクスチャ
			m_albedoTex = tex;
			m_pAlbedoTex = m_albedoTex;
		}
		//テクスチャ変更
		m_defaultMaterialSetting.SetAlbedoTexture(tex);		
	}
	//アルベドテクスチャをデフォに戻す
	void SetDefaultAlbedoTexture() {
		m_defaultMaterialSetting.SetDefaultAlbedoTexture();
	}
	//デフォルトのアルベドテクスチャを取得
	ID3D11ShaderResourceView* GetDefaultAlbedoTexture() const{
		return m_albedoTex;
	}

	//シェーダを設定
	void SetPS(Shader* ps) {
		m_defaultMaterialSetting.SetPS(ps);
	}
	//シェーダをデフォに戻す
	void SetDefaultPS() {
		m_defaultMaterialSetting.SetDefaultPS();
	}
	//デフォルトのシェーダを取得
	Shader* GetDefaultPS() {
		return &m_psDefaultShader[enALL];
	}

	//名前を設定
	void SetMatrialName(const wchar_t* matName)
	{
		m_defaultMaterialSetting.SetMatrialName(matName);
	}
	//名前を取得
	const wchar_t* GetMatrialName()const {
		return m_defaultMaterialSetting.GetMatrialName();
	}
	//名前の一致を判定
	bool EqualMaterialName(const wchar_t* name) const
	{
		return m_defaultMaterialSetting.EqualMaterialName(name);
	}

	//ライティングするかを設定
	void SetLightingEnable(bool enable) {
		m_defaultMaterialSetting.SetLightingEnable(enable);
	}
	//自己発光色(エミッシブ)を設定
	void SetEmissive(const CVector3& emissive) {
		m_defaultMaterialSetting.SetEmissive(emissive);
	}
	//アルベドにかけるスケールを設定
	void SetAlbedoScale(const CVector4& scale) {
		m_defaultMaterialSetting.SetAlbedoScale(scale);
	}


	//これより下、内部用

	//使うマテリアル設定を適応
	void SetUseMaterialSetting(MaterialSetting& matset) {
		m_materialParam = matset.GetMaterialParam();
		m_pPSShader = matset.GetPS();
		m_pAlbedoTex = matset.GetAlbedoTexture();

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
	//動的リンク
	void LoadClassInstanceVS(){
		/*
		//オフセット取得
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(m_vsDefaultShader.GetByteCode(), m_vsDefaultShader.GetByteCodeSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		ID3D11ShaderReflectionVariable* pAmbientLightingVar = pReflector->GetVariableByName("g_calcOldPos");
		m_clacOldPosOffset = pAmbientLightingVar->GetInterfaceSlot(0);

		pReflector->Release();

		//インスタンス取得
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cCalcOldPos", 0, 0, 0, 0, &m_cCalcOldPos);
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cNotCalcOldPos", 0, 0, 0, 0, &m_cNoCalcOldPos);
		*/
	}
	void LoadClassInstancePS() {
		/*
		//オフセット取得
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(m_psDefaultShader.GetByteCode(), m_psDefaultShader.GetByteCodeSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		ID3D11ShaderReflectionVariable* pAmbientLightingVar = pReflector->GetVariableByName("g_calcVelocity");
		m_clacVelocityOffset = pAmbientLightingVar->GetInterfaceSlot(0);

		pReflector->Release();

		//インスタンス取得
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cCalcVelocity", 0, 0, 0, 0, &m_cCalcVelocity);
		ShaderResources::GetInstance().GetClassLinkage()->CreateClassInstance("cNotCalcVelocity", 0, 0, 0, 0, &m_cNoCalcVelocity);
		*/
	}
};
/*!
*@brief
*  スキンなしモデルエフェクト。
*/
class NonSkinModelEffect : public ModelEffect {
public:
	NonSkinModelEffect()
	{
		D3D_SHADER_MACRO macros[] = { "MOTIONBLUR", "1", NULL, NULL };
		m_vsDefaultShader[enALL].Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "ALL", macros);
		m_vsDefaultShader[enNoMotionBlur].Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();
		
		m_pVSShader = &m_vsDefaultShader[enALL];
		isSkining = false;
	}
};
/*!
*@brief
*  スキンモデルエフェクト。
*/
class SkinModelEffect : public ModelEffect {
public:
	SkinModelEffect()
	{
		wchar_t hoge[256];
		GetCurrentDirectoryW(256, hoge);
		D3D_SHADER_MACRO macros[] = { "MOTIONBLUR", "1", NULL, NULL };
		m_vsDefaultShader[enALL].Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, "ALL", macros);
		m_vsDefaultShader[enNoMotionBlur].Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();

		m_pVSShader = &m_vsDefaultShader[enALL];
		isSkining = true;
	}
};

/*!
*@brief
*  エフェクトファクトリ。
*/
class SkinModelEffectFactory : public DirectX::EffectFactory {
public:
	SkinModelEffectFactory(ID3D11Device* device) :
		EffectFactory(device) {}
	std::shared_ptr<DirectX::IEffect> __cdecl CreateEffect(const EffectInfo& info, ID3D11DeviceContext* deviceContext)override
	{
		//モデルエフェクト作成
		std::shared_ptr<ModelEffect> effect;
		if (info.enableSkinning) {
			//スキニングあり。
			effect = std::make_shared<SkinModelEffect>();
		}
		else {
			//スキニングなし。
			effect = std::make_shared<NonSkinModelEffect>();
		}

		//名前設定
		effect->GetDefaultMaterialSetting().SetMatrialName(info.name);

		//テクスチャ設定
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