#pragma once

namespace DemolisherWeapon {

	class ModelEffect;

	//�萔�o�b�t�@�@[model.fx:MaterialCb]
	//�}�e���A���p�����[�^�[
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();		//�A���x�h�ɂ�����X�P�[��
		float emissive = 0.0f;	//���Ȕ���
		float isLighting = 1.0f;	//���C�e�B���O���邩
		float uvOffset[2] = { 0.0f,0.0f };
	};

	class MaterialSetting
	{
	public:

		~MaterialSetting(){
			if (m_pAlbedoTex) { m_pAlbedoTex->Release(); m_pAlbedoTex = nullptr; }
			if (m_pNormalTex) { m_pNormalTex->Release(); m_pNormalTex = nullptr; }
		}

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
			m_materialParam.isLighting = enable ? 1.0f : 0.0f;
		}

		//���Ȕ����F(�G�~�b�V�u)��ݒ�
		void SetEmissive(float emissive) {
			m_materialParam.emissive = emissive;
		}

		//�A���x�h�ɂ�����X�P�[����ݒ�
		void SetAlbedoScale(const CVector4& scale) {
			m_materialParam.albedoScale = scale;
		}

		//UV�I�t�Z�b�g��ݒ�
		void SetUVOffset(const CVector2& uv){
			m_materialParam.uvOffset[0] = uv.x;
			m_materialParam.uvOffset[1] = uv.y;
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
		Shader* GetVS() const {
			return m_pVSShader;
		}
		Shader* GetVSZ() const {
			return m_pVSZShader;
		}
		Shader* GetPS() const{
			return m_pPSShader;
		}
		//�V�F�[�_��ݒ�
		void SetVS(Shader* ps) {
			m_pVSShader = ps;
		}
		void SetVSZ(Shader* ps) {
			m_pVSZShader = ps;
		}
		void SetPS(Shader* ps) {
			m_pPSShader = ps;
		}
		//�V�F�[�_���f�t�H�ɖ߂�
		void SetDefaultVS();
		void SetDefaultVSZ();
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

		//�m�[�}���}�b�v���擾
		ID3D11ShaderResourceView* GetNormalTexture()const {
			return m_pNormalTex;
		}
		//�m�[�}���}�b�v��ݒ�
		void SetNormalTexture(ID3D11ShaderResourceView* tex) {

			if (m_pNormalTex == tex) { return; }//����

			if (m_pNormalTex) {
				m_pNormalTex->Release();
			}
			m_pNormalTex = tex;
			m_pNormalTex->AddRef();
		}

		//���[�V�����u���[�L������ݒ�
		void SetIsMotionBlur(bool enable) {
			m_enableMotionBlur = enable;
		}
		//���[�V�����u���[�L�������擾
		bool GetIsMotionBlur() const{
			return m_enableMotionBlur;
		}

		//ZShader�Ńe�N�X�`���g����
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

		std::wstring m_materialName;							//�}�e���A����
		MaterialParam m_materialParam;							//�}�e���A���p�����[�^
		Shader *m_pVSShader = nullptr, *m_pVSZShader = nullptr; //���_�V�F�[�_
		Shader *m_pPSShader = nullptr;							//�s�N�Z���V�F�[�_
		ID3D11ShaderResourceView* m_pAlbedoTex = nullptr;		//�e�N�X�`��
		ID3D11ShaderResourceView* m_pNormalTex = nullptr;

		bool m_enableMotionBlur = true;
		bool m_isUseTexZShader = false;
	};

}