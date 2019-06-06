#pragma once

#include "Graphic/Shader/Shader.h"
#include "Graphic/Shader/ShaderUtil.h"
#include "Graphic/Shader/ShaderResources.h"

#include "SkinModelEffectShader.h"

#include "MaterialSetting.h"

#define BIT(x) (1<<(x))

namespace DemolisherWeapon {

#include "Preset/Shader/ShadowMapHeader.h"

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
	//使用中のシェーダーのポインタ
	SKEShaderPtr m_pVSShader; Shader* m_pVSZShader = nullptr;
	SKEShaderPtr m_pPSShader; Shader* m_pPSZShader = nullptr;

	//デフォルトバーテックスシェーダ
	SkinModelEffectShader m_vsDefaultShader;
	Shader m_vsZShader;//Z値出力用
	//int m_clacOldPosOffset = 0;
	//ID3D11ClassInstance* m_cCalcOldPos = nullptr, *m_cNoCalcOldPos = nullptr;

	//デフォルトピクセルシェーダ
	bool m_isUseTexZShader = false;
	SkinModelEffectShader m_psDefaultShader;
	Shader m_psZShader[2];//Z値出力用
	SkinModelEffectShader m_psTriPlanarMapShader;//TriPlanarMapping用のシェーダ
	//int m_clacVelocityOffset = 0;
	//ID3D11ClassInstance* m_cCalcVelocity = nullptr, *m_cNoCalcVelocity = nullptr;	

	bool isSkining;//スキンモデルか？

	//テクスチャ
	ID3D11ShaderResourceView* m_defaultAlbedoTex = nullptr, *m_pAlbedoTex = nullptr;
	ID3D11ShaderResourceView* m_defaultNormalTex = nullptr, *m_pNormalTex = nullptr;
	ID3D11ShaderResourceView* m_defaultLightingTex = nullptr, *m_pLightingTex = nullptr;
	
	MaterialSetting m_defaultMaterialSetting;	//マテリアル設定
	MaterialParam m_materialParam;				//マテリアルパラメータ
	ID3D11Buffer* m_materialParamCB = nullptr;	//マテリアルパラメータ用の定数バッファ

	bool m_enableMotionBlur = true;
public:
	ModelEffect()
	{
		//マクロごとにピクセルシェーダを作成
		m_psDefaultShader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
		m_psTriPlanarMapShader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
		
		//マクロごとにZ値描画ピクセルシェーダを作成
		D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
		m_psZShader[0].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
		m_psZShader[1].Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		
		LoadClassInstancePS();

		//デフォルトのシェーダを設定
		m_pPSShader = &m_psDefaultShader;
		m_pPSZShader = &m_psZShader[0];

		//マテリアル設定(m_defaultMaterialSetting)を初期化してやる
		MaterialSettingInit(m_defaultMaterialSetting);

		//マテリアルパラメーターの定数バッファ
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
		*pShaderByteCode = m_vsDefaultShader.GetShader(SkinModelEffectShader::enALL).GetByteCode();
		*pByteCodeLength = m_vsDefaultShader.GetShader(SkinModelEffectShader::enALL).GetByteCodeSize();
	}

	//スキンモデルかどうか取得
	bool GetIsSkining()const {
		return isSkining;
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

	/// <summary>
	/// アルベドテクスチャを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	/// <param name="defaultTex">デフォルトテクスチャとして設定するか?</param>
	void SetAlbedoTexture(ID3D11ShaderResourceView* tex, bool defaultTex = false){
		if (!m_defaultAlbedoTex && defaultTex) {
			//デフォルトテクスチャ
			m_defaultAlbedoTex = tex;
			m_pAlbedoTex = m_defaultAlbedoTex;
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
		return m_defaultAlbedoTex;
	}

	/// <summary>
	/// ノーマルマップを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	/// <param name="defaultTex">デフォルトテクスチャとして設定するか?</param>
	void SetNormalTexture(ID3D11ShaderResourceView* tex, bool defaultTex = false) {
		if (!m_defaultNormalTex && defaultTex) {
			//デフォルトテクスチャ
			m_defaultNormalTex = tex;
			m_pNormalTex = m_defaultNormalTex;
		}
		//テクスチャ変更
		m_defaultMaterialSetting.SetNormalTexture(tex);
	}
	//ノーマルマップをデフォに戻す
	void SetDefaultNormalTexture() {
		m_defaultMaterialSetting.SetDefaultNormalTexture();
	}
	//デフォルトのノーマルマップを取得
	ID3D11ShaderResourceView* GetDefaultNormalTexture() const {
		return m_defaultNormalTex;
	}

	/// <summary>
	/// ライティングパラメータマップを設定
	/// </summary>
	/// <param name="tex">設定するテクスチャ</param>
	/// <param name="defaultTex">デフォルトテクスチャとして設定するか?</param>
	void SetLightingTexture(ID3D11ShaderResourceView* tex, bool defaultTex = false) {
		if (!m_defaultLightingTex && defaultTex) {
			//デフォルトテクスチャ
			m_defaultLightingTex = tex;
			m_pLightingTex = m_defaultLightingTex;
		}
		//テクスチャ変更
		m_defaultMaterialSetting.SetLightingTexture(tex);
	}
	//ライティングパラメータマップをデフォに戻す
	void SetDefaultLightingTexture() {
		m_defaultMaterialSetting.SetDefaultLightingTexture();
	}
	//デフォルトのライティングパラメータマップを取得
	ID3D11ShaderResourceView* GetDefaultLightingTexture() const {
		return m_defaultLightingTex;
	}

	//シェーダを設定
	void SetVS(Shader* vs) {
		m_defaultMaterialSetting.SetVS(vs);
	}
	void SetVSZ(Shader* vs) {
		m_defaultMaterialSetting.SetVSZ(vs);
	}
	void SetPS(Shader* ps) {
		m_defaultMaterialSetting.SetPS(ps);
	}
	//シェーダをデフォに戻す
	void SetDefaultVS() {
		m_defaultMaterialSetting.SetDefaultVS();
	}
	void SetDefaultVSZ() {
		m_defaultMaterialSetting.SetDefaultVSZ();
	}
	void SetDefaultPS() {
		m_defaultMaterialSetting.SetDefaultPS();
	}
	//デフォルトのシェーダを取得
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
	//TriPlanarMapping用のシェーダを取得
	SkinModelEffectShader* GetTriPlanarMappingPS() {
		return &m_psTriPlanarMapShader;
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
	void SetEmissive(float emissive) {
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
		
		m_pVSShader = matset.GetVS();
		m_pVSZShader = matset.GetVSZ(); 
		m_pPSShader = matset.GetPS();
		m_pPSZShader = matset.GetPSZ();

		m_pAlbedoTex = matset.GetAlbedoTexture();
		m_pNormalTex = matset.GetNormalTexture();
		m_pLightingTex = matset.GetLightingTexture();

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
		//マクロごとに頂点シェーダを作成
		m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
		
		//Z値描画シェーダを作成
		m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();
		
		//デフォルトのシェーダを設定
		m_pVSShader = &m_vsDefaultShader;
		m_pVSZShader = &m_vsZShader;

		//スキンモデルじゃない
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
		//マクロごとに頂点シェーダを作成
		m_vsDefaultShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
		
		//Z値描画シェーダを作成
		m_vsZShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);

		LoadClassInstanceVS();

		//デフォルトのシェーダを設定
		m_pVSShader = &m_vsDefaultShader;
		m_pVSZShader = &m_vsZShader;

		//スキンモデルである
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

		//アルベド
		if (info.diffuseTexture && *info.diffuseTexture)
		{
			//テクスチャで設定
			ID3D11ShaderResourceView* texSRV;
			DirectX::EffectFactory::CreateTexture(info.diffuseTexture, deviceContext, &texSRV);
			effect->SetAlbedoTexture(texSRV,true);
		}
		else {
			//ディフューズカラー(数値)で設定
			effect->SetAlbedoScale({ info.diffuseColor.x,info.diffuseColor.y,info.diffuseColor.z,1.0f });
		}

		//ノーマル
		if (info.normalTexture && *info.normalTexture)
		{
			//テクスチャで設定
			ID3D11ShaderResourceView* texSRV;
			DirectX::EffectFactory::CreateTexture(info.normalTexture, deviceContext, &texSRV);
			effect->SetNormalTexture(texSRV, true);
		}

		//ライティングパラメータ
		if (info.specularTexture && *info.specularTexture)
		{
			//テクスチャで設定
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