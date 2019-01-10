#pragma once

namespace DemolisherWeapon {

	//�萔�o�b�t�@�@[model.fx:MaterialCb]
	//�}�e���A���p�����[�^�[
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();	//�A���x�h�ɂ�����X�P�[��
		CVector3 emissive;						//�G�~�b�V�u(���Ȕ���)
		int isLighting = 1;						//���C�e�B���O���邩
	};

	class MaterialSetting
	{
	public:

		void Init(const wchar_t* matName, const MaterialParam& param) {
			SetMatrialName(matName);
			SetMaterialParam(param);
		}

		//���O��ݒ�
		void SetMatrialName(const wchar_t* matName)
		{
			m_materialName = matName;
		}
		//���O���擾
		const wchar_t* GetMatrialName()const {
			return m_materialName.c_str();
		}
		//���O�̈�v�𔻒�
		bool EqualMaterialName(const wchar_t* name) const
		{
			return wcscmp(name, m_materialName.c_str()) == 0;
		}

		//���C�e�B���O���邩��ݒ�
		void SetLightingEnable(bool enable) {
			m_materialParam.isLighting = enable ? 1 : 0;
		}

		//���Ȕ����F(�G�~�b�V�u)��ݒ�
		void SetEmissive(const CVector3& emissive) {
			m_materialParam.emissive = emissive;
		}

		//�A���x�h�ɂ�����X�P�[����ݒ�
		void SetAlbedoScale(const CVector4& scale) {
			m_materialParam.albedoScale = scale;
		}

		//�}�e���A���p�����[�^�擾
		const MaterialParam& GetMaterialParam()const {
			return m_materialParam;
		}
		//�}�e���A���p�����[�^�ݒ�
		void SetMaterialParam(const MaterialParam& param){
			m_materialParam = param;
		}

	private:
		std::wstring m_materialName;			//!<�}�e���A�����B
		MaterialParam m_materialParam;	//�}�e���A���p�����[�^
	};

}