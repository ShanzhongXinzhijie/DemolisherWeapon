#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"

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
	Shader m_vsShader, m_vsZShader;
	Shader m_psShader, m_psZShader;
	bool isSkining;
	ID3D11ShaderResourceView* m_albedoTex = nullptr, *m_pAlbedoTex = nullptr;
	
	MaterialSetting m_materialSetting;			//マテリアル設定
	MaterialParam m_materialParam;				//マテリアルパラメータ
	ID3D11Buffer* m_materialParamCB = nullptr;	//マテリアルパラメータ用の定数バッファ

public:
	ModelEffect()
	{
		m_psShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psZShader.Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);

		m_pPSShader = &m_psShader;

		MaterialSettingInit(m_materialSetting);

		//マテリアルパラメーターの定数バッファ
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

	//マテリアル設定を初期化してやる
	void MaterialSettingInit(MaterialSetting& matset) {
		matset.Init(this);
		matset.SetMatrialName(GetMatrialName());
		matset.SetMaterialParam(m_materialSetting.GetMaterialParam());
		matset.SetPS(m_materialSetting.GetPS());
		matset.SetAlbedoTexture(m_materialSetting.GetAlbedoTexture());
	}
	//マテリアル設定の取得
	MaterialSetting& GetMaterialSetting() {
		return m_materialSetting;
	}

	//アルベドテクスチャを設定
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex)
	{
		if (!m_albedoTex) {
			//デフォルトテクスチャ
			m_albedoTex = tex;
			m_pAlbedoTex = m_albedoTex;
		}

		//テクスチャ変更
		m_materialSetting.SetAlbedoTexture(tex);		
	}
	//アルベドテクスチャをデフォに戻す
	void SetDefaultAlbedoTexture() {
		m_materialSetting.SetDefaultAlbedoTexture();
	}
	//デフォルトのアルベドテクスチャを取得
	ID3D11ShaderResourceView* GetDefaultAlbedoTexture() const{
		return m_albedoTex;
	}

	//シェーダを設定
	void SetPS(Shader* ps) {
		m_materialSetting.SetPS(ps);
	}
	//シェーダをデフォに戻す
	void SetDefaultPS() {
		m_materialSetting.SetDefaultPS();
	}
	//デフォルトのシェーダを取得
	Shader* GetDefaultPS() {
		return &m_psShader;
	}

	//名前を設定
	void SetMatrialName(const wchar_t* matName)
	{
		m_materialSetting.SetMatrialName(matName);
	}
	//名前を取得
	const wchar_t* GetMatrialName()const {
		return m_materialSetting.GetMatrialName();
	}
	//名前の一致を判定
	bool EqualMaterialName(const wchar_t* name) const
	{
		return m_materialSetting.EqualMaterialName(name);
	}

	//ライティングするかを設定
	void SetLightingEnable(bool enable) {
		m_materialSetting.SetLightingEnable(enable);
	}
	//自己発光色(エミッシブ)を設定
	void SetEmissive(const CVector3& emissive) {
		m_materialSetting.SetEmissive(emissive);
	}
	//アルベドにかけるスケールを設定
	void SetAlbedoScale(const CVector4& scale) {
		m_materialSetting.SetAlbedoScale(scale);
	}


	//これより下、内部用

	//使うマテリアル設定を適応
	void SetUseMaterialSetting(MaterialSetting& matset) {
		m_materialParam = matset.GetMaterialParam();
		m_pPSShader = matset.GetPS();
		m_pAlbedoTex = matset.GetAlbedoTexture();
	}
	void SetDefaultMaterialSetting() {
		m_materialParam = m_materialSetting.GetMaterialParam();
		m_pPSShader = m_materialSetting.GetPS();
		m_pAlbedoTex = m_materialSetting.GetAlbedoTexture();
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
		m_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);
		
		m_pVSShader = &m_vsShader;
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
		m_vsShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		m_pVSShader = &m_vsShader;
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
		effect->GetMaterialSetting().SetMatrialName(info.name);

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