#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"
#include "Graphic/Shader/ShaderResources.h"

#include "SkinModelEffectShader.h"

#include "MaterialSetting.h"

namespace DemolisherWeapon {

#include "Preset/Shader/ShadowMapHeader.h"

/*!
*@brief	モデルエフェクト。
*/
class ModelEffect : public DirectX::IEffect {
protected:
	/*
	//デフォルトバーテックスシェーダ
	SkinModelEffectShader m_vsDefaultShader;
	Shader m_vsZShader;//Z値出力用

	//デフォルトピクセルシェーダ
	SkinModelEffectShader m_psDefaultShader;
	Shader m_psZShader[2];//Z値出力用
	SkinModelEffectShader m_psTriPlanarMapShader, m_psTriPlanarMapShaderYOnly;//TriPlanarMapping用のシェーダ

	bool isSkining;//スキンモデルか？

	//テクスチャ
	ID3D11ShaderResourceView* m_defaultAlbedoTex = nullptr;
	ID3D11ShaderResourceView* m_defaultNormalTex = nullptr;
	ID3D11ShaderResourceView* m_defaultLightingTex = nullptr;
	
	MaterialSetting* m_ptrUseMaterialSetting = nullptr;	//使用するマテリアル設定
	MaterialSetting m_defaultMaterialSetting;	//マテリアル設定(デフォルト)
	ID3D11Buffer* m_materialParamCB = nullptr;	//マテリアルパラメータ用の定数バッファ
	*/
	MaterialData m_matData;

public:
	ModelEffect()
	{
		/*
		//マクロごとにピクセルシェーダを作成
		m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psTriPlanarMapShader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
		D3D_SHADER_MACRO macrosYOnly[] = { "Y_ONLY", "1", NULL, NULL };
		m_psTriPlanarMapShaderYOnly.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS, "Y_ONLY", macrosYOnly);

		//マクロごとにZ値描画ピクセルシェーダを作成
		D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
		m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
		m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		
		//マテリアル設定(m_defaultMaterialSetting)を初期化してやる
		MaterialSettingInit(m_defaultMaterialSetting);

		//使用するマテリアル設定をセット
		SetDefaultMaterialSetting();

		//マテリアルパラメーターの定数バッファ
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

	//スキンモデルかどうか取得
	bool GetIsSkining()const {
		return m_matData.m_isSkining;
	}

	//マテリアル設定をこのインスタンスをベースに初期化してやる
	//MaterialSetting& matset　初期化するセッティング
	void MaterialSettingInit(MaterialSetting& matset) {
		matset.Init(&m_matData);
	}
	//デフォルトマテリアル設定の取得
	MaterialSetting& GetDefaultMaterialSetting() {
		return m_matData.m_defaultMaterialSetting;
	}

	/// <summary>
	/// アルベドテクスチャを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex){
		//テクスチャ変更
		m_matData.m_defaultMaterialSetting.SetAlbedoTexture(tex);
	}
	//アルベドテクスチャをデフォに戻す
	void SetDefaultAlbedoTexture() {
		m_matData.m_defaultMaterialSetting.SetDefaultAlbedoTexture();
	}
	//デフォルトのアルベドテクスチャを取得
	ID3D11ShaderResourceView* GetDefaultAlbedoTexture() const {
		return m_matData.m_albedo.textureView.Get();
	}

	/// <summary>
	/// ノーマルマップを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	void SetNormalTexture(ID3D11ShaderResourceView* tex) {
		//テクスチャ変更
		m_matData.m_defaultMaterialSetting.SetNormalTexture(tex);
	}
	//ノーマルマップをデフォに戻す
	void SetDefaultNormalTexture() {
		m_matData.m_defaultMaterialSetting.SetDefaultNormalTexture();
	}
	//デフォルトのノーマルマップを取得
	ID3D11ShaderResourceView* GetDefaultNormalTexture() const {
		return m_matData.m_normal.textureView.Get();
	}

	/// <summary>
	/// ライティングパラメータマップを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	void SetLightingTexture(ID3D11ShaderResourceView* tex) {
		//テクスチャ変更
		m_matData.m_defaultMaterialSetting.SetLightingTexture(tex);
	}
	//ライティングパラメータマップをデフォに戻す
	void SetDefaultLightingTexture() {
		m_matData.m_defaultMaterialSetting.SetDefaultLightingTexture();
	}
	//デフォルトのライティングパラメータマップを取得
	ID3D11ShaderResourceView* GetDefaultLightingTexture() const {
		return m_matData.m_lighting.textureView.Get();
	}

	/// <summary>
	/// トランスルーセントマップを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {
		m_matData.m_defaultMaterialSetting.SetTranslucentTexture(tex);
	}

	//シェーダを設定
	void SetVS(Shader* vs) {
		m_matData.m_defaultMaterialSetting.SetVS(vs);
	}
	void SetVSZ(Shader* vs) {
		m_matData.m_defaultMaterialSetting.SetVSZ(vs);
	}
	void SetPS(Shader* ps) {
		m_matData.m_defaultMaterialSetting.SetPS(ps);
	}
	//シェーダをデフォに戻す
	void SetDefaultVS() {
		m_matData.m_defaultMaterialSetting.SetDefaultVS();
	}
	void SetDefaultVSZ() {
		m_matData.m_defaultMaterialSetting.SetDefaultVSZ();
	}
	void SetDefaultPS() {
		m_matData.m_defaultMaterialSetting.SetDefaultPS();
	}
	//デフォルトのシェーダを取得
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
	//TriPlanarMapping用のシェーダを取得
	SkinModelEffectShader* GetTriPlanarMappingPS(bool isYOnly) {
		return isYOnly ? &m_matData.m_psTriPlanarMapShaderYOnly : &m_matData.m_psTriPlanarMapShader;
	}

	//名前を設定
	void SetMatrialName(const wchar_t* matName)
	{
		m_matData.m_defaultMaterialSetting.SetMatrialName(matName);
	}
	//名前を取得
	const wchar_t* GetMatrialName()const {
		return m_matData.m_defaultMaterialSetting.GetMatrialName();
	}
	//名前の一致を判定
	bool EqualMaterialName(const wchar_t* name) const
	{
		return m_matData.m_defaultMaterialSetting.EqualMaterialName(name);
	}

	//ライティングするかを設定
	void SetLightingEnable(bool enable) {
		m_matData.m_defaultMaterialSetting.SetLightingEnable(enable);
	}
	//自己発光色(エミッシブ)を設定
	void SetEmissive(float emissive) {
		m_matData.m_defaultMaterialSetting.SetEmissive(emissive);
	}
	//アルベドにかけるスケールを設定
	void SetAlbedoScale(const CVector4& scale) {
		m_matData.m_defaultMaterialSetting.SetAlbedoScale(scale);
	}


	//これより下、内部用

	//使うマテリアル設定を適応
	void SetUseMaterialSetting(MaterialSetting& matset) {
		m_matData.m_ptrUseMaterialSetting = &matset;
	}
	void SetDefaultMaterialSetting() {
		SetUseMaterialSetting(m_matData.m_defaultMaterialSetting);
	}
	/// <summary>
	/// マテリアルデータを取得
	/// </summary>
	MaterialData& GetMatrialData() {
		return m_matData;
	}
};

/*!
*@brief
*  エフェクトファクトリ。
*/
#ifndef DW_DX12_TEMPORARY
class SkinModelEffectFactory : public DirectX::EffectFactory {
public:
	SkinModelEffectFactory(ID3D11Device* device) : EffectFactory(device) {}

	std::shared_ptr<DirectX::IEffect> __cdecl CreateEffect(const EffectInfo& info, ID3D11DeviceContext* deviceContext)override
	{
		//モデルエフェクト作成
		std::shared_ptr<ModelEffect> effect = std::make_shared<ModelEffect>();
		
		wchar_t fullName[MAX_PATH] = {};
		
		//アルベド
		if (info.diffuseTexture && *info.diffuseTexture)
		{
			//テクスチャで設定
			wcscpy_s(fullName, GetDirectory());
			wcscat_s(fullName, info.diffuseTexture);
			effect->GetMatrialData().InitAlbedoTexture(fullName);
		}
		else {
			//ディフューズカラー(数値)で設定
			effect->GetMatrialData().InitAlbedoColor({ info.diffuseColor.x,info.diffuseColor.y,info.diffuseColor.z });
		}

		//ノーマル
		if (info.normalTexture && *info.normalTexture)
		{
			//テクスチャで設定
			wcscpy_s(fullName, GetDirectory());
			wcscat_s(fullName, info.normalTexture);
			effect->GetMatrialData().InitNormalTexture(fullName);
		}

		//ライティングパラメータ
		if (info.specularTexture && *info.specularTexture)
		{
			//テクスチャで設定
			wcscpy_s(fullName, GetDirectory());
			wcscat_s(fullName, info.specularTexture);
			effect->GetMatrialData().InitLightingTexture(fullName);
		}

		//初期化
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