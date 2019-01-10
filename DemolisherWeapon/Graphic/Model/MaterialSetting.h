#pragma once

namespace DemolisherWeapon {

	//定数バッファ　[model.fx:MaterialCb]
	//マテリアルパラメーター
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();	//アルベドにかけるスケール
		CVector3 emissive;						//エミッシブ(自己発光)
		int isLighting = 1;						//ライティングするか
	};

	class MaterialSetting
	{
	public:

		void Init(const wchar_t* matName, const MaterialParam& param) {
			SetMatrialName(matName);
			SetMaterialParam(param);
		}

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
			m_materialParam.isLighting = enable ? 1 : 0;
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

	private:
		std::wstring m_materialName;			//!<マテリアル名。
		MaterialParam m_materialParam;	//マテリアルパラメータ
	};

}