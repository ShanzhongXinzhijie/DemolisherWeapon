#pragma once
#include "tktkmfile.h"
#include "Graphic/Factory/TextureFactory.h"
#include "Graphic/Model/SkinModelEffectShader.h"
//#include "Graphic/shader/ConstantBuffer.h"

namespace DemolisherWeapon {

	class ModelEffect;
	class IMaterial;
	class MaterialData;
	template<class T>class ConstantBufferDx12;

	/// <summary>
	/// マテリアルパラメーター
	/// 定数バッファ　[model.fx:MaterialCb]
	/// </summary>
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();	//アルベドにかけるスケール
		float emissive = 0.0f;					//自己発光
		float isLighting = 1.0f;				//ライティングするか
		float metallic = 0.0f;					//メタリック
		float shininess = 0.38f;				//シャイネス(ラフネスの逆)
		float uvOffset[2] = { 0.0f,0.0f };		//UV座標オフセット
		float triPlanarMapUVScale = 0.005f;		//TriPlanarMapping時のUV座標へのスケール
		float translucent = 0.0f;				//トランスルーセント(光の透過具合)
	};

	/// <summary>
	/// マテリアル設定
	/// </summary>
	class MaterialSetting
	{
	public:
		MaterialSetting() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="modeleffect">親モデルエフェクト</param>
		//void Init(ModelEffect* modeleffect);
		void Init(MaterialData* materialData);

		//名前を設定
		void SetMatrialName(const wchar_t* matName)
		{
			m_materialName = matName;
		}
		//名前を取得
		const wchar_t* GetMatrialName()const {
			return m_materialName.c_str();
		}
		//名前の一致を判定
		bool EqualMaterialName(const wchar_t* name) const
		{
			return wcscmp(name, m_materialName.c_str()) == 0;
		}

		//ライティングするかを設定
		void SetLightingEnable(bool enable) {
			m_materialParam.isLighting = enable ? 1.0f : 0.0f;
		}

		//自己発光色(エミッシブ)を設定
		void SetEmissive(float emissive) {
			m_materialParam.emissive = emissive;
		}
		/// <summary>
		/// メタリックを設定
		/// </summary>
		/// <param name="metallic">基本的に1か0のどっちかに設定する</param>
		void SetMetallic(float metallic) {
			m_materialParam.metallic = metallic;
		}
		/// <summary>
		/// シャイネス(ラフネスの逆)を設定
		/// </summary>
		/// <param name="shininess">値が高いほど材質がつるつる(0.0～1.0)</param>
		void SetShininess(float shininess) {
			m_materialParam.shininess = shininess;
		}

		//アルベドにかけるスケールを設定
		void SetAlbedoScale(const CVector4& scale) {
			m_materialParam.albedoScale = scale;
		}		

		//UVオフセットを設定
		void SetUVOffset(const CVector2& uv){
			m_materialParam.uvOffset[0] = uv.x;
			m_materialParam.uvOffset[1] = uv.y;
		}

		/// <summary>
		/// トランスルーセントを設定
		/// </summary>
		/// <param name="translucent">0.0f～1.0f 値が高いほど光を透過する</param>
		void SetTranslucent(float translucent) {
			m_materialParam.translucent = translucent;
		}

		//マテリアルパラメータ取得
		const MaterialParam& GetMaterialParam()const {
			return m_materialParam;
		}
		//マテリアルパラメータ設定
		void SetMaterialParam(const MaterialParam& param){
			m_materialParam = param;
		}

		//シェーダを取得
		const SKEShaderPtr& GetVS() const {
			return m_pVSShader;
		}
		Shader* GetVSZ() const {
			return m_pVSZShader;
		}
		const SKEShaderPtr& GetPS() const{
			return m_pPSShader;
		}
		Shader* GetPSZ() const {
			return m_pPSZShader;
		}
		//シェーダを設定
		void SetVS(const SKEShaderPtr& ps) {
			m_pVSShader = ps;
		}
		void SetVSZ(Shader* ps) {
			m_pVSZShader = ps;
		}
		void SetPS(const SKEShaderPtr& ps) {
			m_pPSShader = ps;
		}
		void SetPSZ(Shader* ps) {
			m_pPSZShader = ps;
		}
		//シェーダをデフォに戻す
		void SetDefaultVS();
		void SetDefaultVSZ();
		void SetDefaultPS();
		void SetDefaultPSZ();

		/// <summary>
		/// TriPlanarMappingシェーダを設定
		/// </summary>
		/// <param name="isYOnly">マッピングを縦方向に限定するか</param>
		void SetTriPlanarMappingPS(bool isYOnly = false);
		//TriPlanarMapping時のUV座標へのスケールを設定
		void SetTriPlanarMappingUVScale(float texScale) {
			m_materialParam.triPlanarMapUVScale = texScale;
		}		

		//アルベドテクスチャを取得
		ID3D11ShaderResourceView* GetAlbedoTexture()const {
			return m_albedo.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfAlbedoTexture()const {
			return m_albedo.textureView.GetAddressOf();
		}
		const TextueData& GetAlbedoTextureData()const {
			return m_albedo;
		}
		//アルベドテクスチャを設定
		void SetAlbedoTexture(ID3D11ShaderResourceView* tex){
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetAlbedoTexture(texData);
		}
		//アルベドテクスチャを設定
		void SetAlbedoTexture(const TextueData& tex) {
			if (m_albedo == tex) { return; }//既に

			if (!m_albedo.isLoaded()) {
				SetAlbedoScale(CVector4::One());//アルベドスケールを初期化
			}

			m_albedo = tex;//コピー
		}
		//アルベドテクスチャをデフォに戻す
		void SetDefaultAlbedoTexture();

		//ノーマルマップを取得
		ID3D11ShaderResourceView* GetNormalTexture()const {
			return m_normal.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfNormalTexture()const {
			return m_normal.textureView.GetAddressOf();
		}
		const TextueData& GetNormalTextureData()const {
			return m_normal;
		}
		//ノーマルマップを設定
		void SetNormalTexture(ID3D11ShaderResourceView* tex) {
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetNormalTexture(texData);
		}
		//ノーマルマップを設定
		void SetNormalTexture(const TextueData& tex) {
			if (m_normal == tex) { return; }//既に
			m_normal = tex;//コピー
		}
		//ノーマルマップをデフォに戻す
		void SetDefaultNormalTexture();

		//ライティングパラメータマップを取得
		ID3D11ShaderResourceView* GetLightingTexture()const {
			return m_lighting.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfLightingTexture()const {
			return m_lighting.textureView.GetAddressOf();
		}
		const TextueData& GetLightingTextureData()const {
			return m_lighting;
		}
		//ライティングパラメータマップを設定
		void SetLightingTexture(ID3D11ShaderResourceView* tex) {
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetLightingTexture(texData);
		}
		//ライティングパラメータマップを設定
		void SetLightingTexture(const TextueData& tex) {
			if (m_lighting == tex) { return; }//既に

			if (!m_lighting.isLoaded()) {
				//初期化(これらのパラメータはテクスチャにかけるスケールとして使う)
				SetEmissive(1.0f);
				SetMetallic(1.0f);
				SetShininess(1.0f);
			}

			m_lighting = tex;//コピー
		}
		//ライティングパラメータマップをデフォに戻す
		void SetDefaultLightingTexture();

		//トランスルーセントマップを取得
		ID3D11ShaderResourceView* GetTranslucentTexture()const {
			return m_translucent.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfTranslucentTexture()const {
			return m_translucent.textureView.GetAddressOf();
		}
		//トランスルーセントマップを設定
		void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetTranslucentTexture(texData);
		}
		//トランスルーセントマップを設定
		void SetTranslucentTexture(const TextueData& tex) {
			if (m_translucent == tex) { return; }//既に

			if (!m_translucent.isLoaded()) {
				//初期化(これらのパラメータはテクスチャにかけるスケールとして使う)
				SetTranslucent(1.0f);
			}

			m_translucent = tex;//コピー
		}

		//モーションブラー有効かを設定
		void SetIsMotionBlur(bool enable) {
			m_enableMotionBlur = enable;
		}
		//モーションブラー有効かを取得
		bool GetIsMotionBlur() const{
			return m_enableMotionBlur;
		}

		//ZShaderでテクスチャ使うか
		void SetIsUseTexZShader(bool is){
			m_isUseTexZShader = is;
		}
		bool GetIsUseTexZShader()const{
			return m_isUseTexZShader;
		}

		//スキンモデルか取得
		bool GetIsSkining()const;

		//ModelEffect* GetModelEffect() {
		//	return m_isInit;
		//}
		
	private:
		MaterialData* m_isInit = nullptr;//親

		std::wstring m_materialName;  //マテリアル名
		MaterialParam m_materialParam;//マテリアルパラメータ

		//頂点シェーダ
		SKEShaderPtr m_pVSShader;								
		Shader *m_pVSZShader = nullptr; 
		//ピクセルシェーダ
		SKEShaderPtr m_pPSShader;		
		Shader *m_pPSZShader = nullptr;

		//テクスチャ
		TextueData m_albedo;
		TextueData m_normal;
		TextueData m_lighting;
		TextueData m_translucent;

		//設定
		bool m_enableMotionBlur = true;
		bool m_isUseTexZShader = false;
	};

	/// <summary>
	/// マテリアルデータ
	/// </summary>
	class MaterialData {
	public:
		MaterialData() = default;

		//初期化
		void Init(bool isSkining, std::wstring_view name);
		void InitAlbedoTexture(std::wstring_view path);
		void InitAlbedoColor(const CVector3& rgb);
		void InitNormalTexture(std::wstring_view path);
		void InitLightingTexture(std::wstring_view path);

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
		Shader* GetDefaultPSZ(bool isTex) {
			return &m_psZShader[isTex ? 1 : 0];
		}
		//TriPlanarMapping用のシェーダを取得
		SkinModelEffectShader* GetTriPlanarMappingPS(bool isYOnly) {
			return isYOnly ? &m_psTriPlanarMapShaderYOnly : &m_psTriPlanarMapShader;
		}

		//デフォルトのアルベドテクスチャを取得
		const TextueData& GetDefaultAlbedoTexture()const {
			return m_defaultMaterialSetting.GetAlbedoTextureData();
		}
		//デフォルトのノーマルマップを取得
		const TextueData& GetDefaultNormalTexture()const {
			return m_defaultMaterialSetting.GetNormalTextureData();
		}
		//デフォルトのライティングパラメータマップを取得
		const TextueData& GetDefaultLightingTexture()const {
			return m_defaultMaterialSetting.GetLightingTextureData();
		}

		//定数バッファの取得(DX11)
		auto& GetConstantBufferDX11() {
			return m_materialParamCBDX11;
		}

		//スキンモデルか取得
		bool GetIsSkining()const {
			return m_isSkining;
		}

		//デフォルトマテリアル設定の取得
		MaterialSetting& GetDefaultMaterialSetting() {
			return m_defaultMaterialSetting;
		}
		//使用中のマテリアル設定の取得
		MaterialSetting& GetUsingMaterialSetting() {
			return *m_ptrUseMaterialSetting;
		}

		//使うマテリアル設定
		void SetUseMaterialSetting(MaterialSetting& matset) {
			m_ptrUseMaterialSetting = &matset;
		}
		//モデルデータデフォルトのマテリアル設定を使用
		void SetDefaultMaterialSetting() {
			SetUseMaterialSetting(m_defaultMaterialSetting);
		}

	private:
		//デフォルトバーテックスシェーダ
		SkinModelEffectShader m_vsDefaultShader;
		Shader m_vsZShader;//Z値出力用

		//デフォルトピクセルシェーダ
		SkinModelEffectShader m_psDefaultShader;
		Shader m_psZShader[2];//Z値出力用
		SkinModelEffectShader m_psTriPlanarMapShader, m_psTriPlanarMapShaderYOnly;//TriPlanarMapping用のシェーダ

		//スキンモデルか？
		bool m_isSkining;
		
		//マテリアル設定
		MaterialSetting* m_ptrUseMaterialSetting = nullptr;	//使用するマテリアル設定
		MaterialSetting m_defaultMaterialSetting;	//マテリアル設定(デフォルト)

		//マテリアルパラメータ用の定数バッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialParamCBDX11;
		std::unique_ptr<ConstantBufferDx12<MaterialParam>> m_materialParamCBDX12;

		//フレンド
		friend class ModelEffect;
	};
}