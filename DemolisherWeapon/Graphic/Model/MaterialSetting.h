#pragma once

namespace DemolisherWeapon {

	class ModelEffect;

	//定数バッファ　[model.fx:MaterialCb]
	//マテリアルパラメーター
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

	class MaterialSetting
	{
	public:

		~MaterialSetting(){
			if (m_pAlbedoTex) { m_pAlbedoTex->Release(); m_pAlbedoTex = nullptr; }
			if (m_pNormalTex) { m_pNormalTex->Release(); m_pNormalTex = nullptr; }
			if (m_pLightingTex) { m_pLightingTex->Release(); m_pLightingTex = nullptr; }
			if (m_pTranslucentTex) { m_pTranslucentTex->Release(); m_pTranslucentTex = nullptr; }			
		}

		void Init(ModelEffect* modeleffect);

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
			return m_pAlbedoTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfAlbedoTexture()const {
			return &m_pAlbedoTex;
		}
		//アルベドテクスチャを設定
		void SetAlbedoTexture(ID3D11ShaderResourceView* tex){

			if (m_pAlbedoTex == tex) { return; }//既に

			if (m_pAlbedoTex) { 
				m_pAlbedoTex->Release();
			}
			else {
				SetAlbedoScale(CVector4::One());//アルベドスケールを初期化
			}
			m_pAlbedoTex = tex;
			if (m_pAlbedoTex) {
				m_pAlbedoTex->AddRef();
			}
		}
		//アルベドテクスチャをデフォに戻す
		void SetDefaultAlbedoTexture();

		//ノーマルマップを取得
		ID3D11ShaderResourceView* GetNormalTexture()const {
			return m_pNormalTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfNormalTexture()const {
			return &m_pNormalTex;
		}
		//ノーマルマップを設定
		void SetNormalTexture(ID3D11ShaderResourceView* tex) {

			if (m_pNormalTex == tex) { return; }//既に

			if (m_pNormalTex) {
				m_pNormalTex->Release();
			}
			m_pNormalTex = tex;
			if (m_pNormalTex) {
				m_pNormalTex->AddRef();
			}
		}
		//ノーマルマップをデフォに戻す
		void SetDefaultNormalTexture();

		//ライティングパラメータマップを取得
		ID3D11ShaderResourceView* GetLightingTexture()const {
			return m_pLightingTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfLightingTexture()const {
			return &m_pLightingTex;
		}
		//ライティングパラメータマップを設定
		void SetLightingTexture(ID3D11ShaderResourceView* tex) {

			if (m_pLightingTex == tex) { return; }//既に

			if (m_pLightingTex) {
				m_pLightingTex->Release();
			}
			else {
				//初期化(これらのパラメータはテクスチャにかけるスケールとして使う)
				SetEmissive(1.0f);
				SetMetallic(1.0f);
				SetShininess(1.0f);
			}
			m_pLightingTex = tex;
			if (m_pLightingTex) {
				m_pLightingTex->AddRef();
			}
		}
		//ライティングパラメータマップをデフォに戻す
		void SetDefaultLightingTexture();

		//トランスルーセントマップを取得
		ID3D11ShaderResourceView* GetTranslucentTexture()const {
			return m_pTranslucentTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfTranslucentTexture()const {
			return &m_pTranslucentTex;
		}
		//トランスルーセントマップを設定
		void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {

			if (m_pTranslucentTex == tex) { return; }//既に

			if (m_pTranslucentTex) {
				m_pTranslucentTex->Release();
			}
			else {
				//初期化(これらのパラメータはテクスチャにかけるスケールとして使う)
				SetTranslucent(1.0f);
			}
			m_pTranslucentTex = tex;
			if (m_pTranslucentTex) {
				m_pTranslucentTex->AddRef();
			}
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

		ModelEffect* GetModelEffect() {
			return m_isInit;
		}
		
	private:
		ModelEffect *m_isInit = nullptr;

		std::wstring m_materialName;  //マテリアル名
		MaterialParam m_materialParam;//マテリアルパラメータ

		//頂点シェーダ
		SKEShaderPtr m_pVSShader;								
		Shader *m_pVSZShader = nullptr; 
		//ピクセルシェーダ
		SKEShaderPtr m_pPSShader;		
		Shader *m_pPSZShader = nullptr;

		//テクスチャ
		ID3D11ShaderResourceView* m_pAlbedoTex = nullptr;		
		ID3D11ShaderResourceView* m_pNormalTex = nullptr;
		ID3D11ShaderResourceView* m_pLightingTex = nullptr;
		ID3D11ShaderResourceView* m_pTranslucentTex = nullptr;

		//設定
		bool m_enableMotionBlur = true;
		bool m_isUseTexZShader = false;
	};

}