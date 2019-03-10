#pragma once

namespace DemolisherWeapon {

	class ModelEffect;

	//定数バッファ　[model.fx:MaterialCb]
	//マテリアルパラメーター
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();		//アルベドにかけるスケール
		CVector4 emissive = {0.0f,0.0f,0.0f,1.0f};	//エミッシブ(自己発光) wがライティングするか
	};

	class MaterialSetting
	{
	public:

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
			m_materialParam.emissive.w = enable ? 1.0f : 0.0f;
		}

		//自己発光色(エミッシブ)を設定
		void SetEmissive(const CVector3& emissive) {
			m_materialParam.emissive = emissive;
		}

		//アルベドにかけるスケールを設定
		void SetAlbedoScale(const CVector4& scale) {
			m_materialParam.albedoScale = scale;
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
		Shader* GetVS() const {
			return m_pVSShader;
		}
		Shader* GetVSZ() const {
			return m_pVSZShader;
		}
		Shader* GetPS() const{
			return m_pPSShader;
		}
		//シェーダを設定
		void SetVS(Shader* ps) {
			m_pVSShader = ps;
		}
		void SetVSZ(Shader* ps) {
			m_pVSZShader = ps;
		}
		void SetPS(Shader* ps) {
			m_pPSShader = ps;
		}
		//シェーダをデフォに戻す
		void SetDefaultVS();
		void SetDefaultVSZ();
		void SetDefaultPS();

		//アルベドテクスチャを取得
		ID3D11ShaderResourceView* GetAlbedoTexture()const {
			return m_pAlbedoTex;
		}
		//アルベドテクスチャを設定
		void SetAlbedoTexture(ID3D11ShaderResourceView* tex){

			if (m_pAlbedoTex == tex) { return; }//既に

			if (m_pAlbedoTex) { 
				m_pAlbedoTex->Release();
			}
			m_pAlbedoTex = tex;
			m_pAlbedoTex->AddRef();
		}
		//アルベドテクスチャをデフォに戻す
		void SetDefaultAlbedoTexture();

		//ノーマルマップを取得
		ID3D11ShaderResourceView* GetNormalTexture()const {
			return m_pNormalTex;
		}
		//ノーマルマップを設定
		void SetNormalTexture(ID3D11ShaderResourceView* tex) {

			if (m_pNormalTex == tex) { return; }//既に

			if (m_pNormalTex) {
				m_pNormalTex->Release();
			}
			m_pNormalTex = tex;
			m_pNormalTex->AddRef();
		}

		//モーションブラー有効かを設定
		void SetIsMotionBlur(bool enable) {
			m_enableMotionBlur = enable;
		}
		//モーションブラー有効かを取得
		bool GetIsMotionBlur() const{
			return m_enableMotionBlur;
		}

		ModelEffect* GetModelEffect() {
			return m_isInit;
		}
		
	private:
		ModelEffect *m_isInit = nullptr;

		std::wstring m_materialName;							//マテリアル名
		MaterialParam m_materialParam;							//マテリアルパラメータ
		Shader *m_pVSShader = nullptr, *m_pVSZShader = nullptr; //頂点シェーダ
		Shader *m_pPSShader = nullptr;							//ピクセルシェーダ
		ID3D11ShaderResourceView* m_pAlbedoTex = nullptr;		//テクスチャ
		ID3D11ShaderResourceView* m_pNormalTex = nullptr;

		bool m_enableMotionBlur = true;
	};

}