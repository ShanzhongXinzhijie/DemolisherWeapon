#pragma once

namespace DemolisherWeapon {

	class ModelEffect;

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

		void Init(ModelEffect* modeleffect);

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

		//�V�F�[�_���擾
		Shader* GetPS() const{
			return m_pPSShader;
		}
		//�V�F�[�_��ݒ�
		void SetPS(Shader* ps) {
			m_pPSShader = ps;
		}
		//�V�F�[�_���f�t�H�ɖ߂�
		void SetDefaultPS();

		//�A���x�h�e�N�X�`�����擾
		ID3D11ShaderResourceView* GetAlbedoTexture()const {
			return m_pAlbedoTex;
		}
		//�A���x�h�e�N�X�`����ݒ�
		void SetAlbedoTexture(ID3D11ShaderResourceView* tex){

			if (m_pAlbedoTex == tex) { return; }//����

			if (m_pAlbedoTex) { 
				m_pAlbedoTex->Release();
			}
			m_pAlbedoTex = tex;
			m_pAlbedoTex->AddRef();
		}
		//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
		void SetDefaultAlbedoTexture();

		//���[�V�����u���[�L������ݒ�
		void SetIsMotionBlur(bool enable) {
			m_isMotionBlur = enable;
		}
		//���[�V�����u���[�L�������擾
		bool GetIsMotionBlur() const{
			return m_isMotionBlur;
		}
		
	private:
		ModelEffect *m_isInit = nullptr;

		std::wstring m_materialName;						//�}�e���A����
		MaterialParam m_materialParam;						//�}�e���A���p�����[�^
		Shader* m_pPSShader = nullptr;						//�V�F�[�_�[
		ID3D11ShaderResourceView* m_pAlbedoTex = nullptr;	//�e�N�X�`��

		bool m_isMotionBlur = true;//���[�V�����u���[
	};

}