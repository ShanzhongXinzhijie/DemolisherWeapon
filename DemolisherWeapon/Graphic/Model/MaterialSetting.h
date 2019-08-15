#pragma once

namespace DemolisherWeapon {

	class ModelEffect;

	//�萔�o�b�t�@�@[model.fx:MaterialCb]
	//�}�e���A���p�����[�^�[
	struct MaterialParam {
		CVector4 albedoScale = CVector4::One();	//�A���x�h�ɂ�����X�P�[��
		float emissive = 0.0f;					//���Ȕ���
		float isLighting = 1.0f;				//���C�e�B���O���邩
		float metallic = 0.0f;					//���^���b�N
		float shininess = 0.38f;				//�V���C�l�X(���t�l�X�̋t)
		float uvOffset[2] = { 0.0f,0.0f };		//UV���W�I�t�Z�b�g
		float triPlanarMapUVScale = 0.005f;		//TriPlanarMapping����UV���W�ւ̃X�P�[��
		float translucent = 0.0f;				//�g�����X���[�Z���g(���̓��ߋ)
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
		/// <summary>
		/// ���^���b�N��ݒ�
		/// </summary>
		/// <param name="metallic">��{�I��1��0�̂ǂ������ɐݒ肷��</param>
		void SetMetallic(float metallic) {
			m_materialParam.metallic = metallic;
		}
		/// <summary>
		/// �V���C�l�X(���t�l�X�̋t)��ݒ�
		/// </summary>
		/// <param name="shininess">�l�������قǍގ������(0.0�`1.0)</param>
		void SetShininess(float shininess) {
			m_materialParam.shininess = shininess;
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

		/// <summary>
		/// �g�����X���[�Z���g��ݒ�
		/// </summary>
		/// <param name="translucent">0.0f�`1.0f �l�������قǌ��𓧉߂���</param>
		void SetTranslucent(float translucent) {
			m_materialParam.translucent = translucent;
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
		//�V�F�[�_��ݒ�
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
		//�V�F�[�_���f�t�H�ɖ߂�
		void SetDefaultVS();
		void SetDefaultVSZ();
		void SetDefaultPS();
		void SetDefaultPSZ();

		//TriPlanarMapping�p�̃V�F�[�_��ݒ�
		void SetTriPlanarMappingPS();
		//TriPlanarMapping����UV���W�ւ̃X�P�[����ݒ�
		void SetTriPlanarMappingUVScale(float texScale) {
			m_materialParam.triPlanarMapUVScale = texScale;
		}		

		//�A���x�h�e�N�X�`�����擾
		ID3D11ShaderResourceView* GetAlbedoTexture()const {
			return m_pAlbedoTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfAlbedoTexture()const {
			return &m_pAlbedoTex;
		}
		//�A���x�h�e�N�X�`����ݒ�
		void SetAlbedoTexture(ID3D11ShaderResourceView* tex){

			if (m_pAlbedoTex == tex) { return; }//����

			if (m_pAlbedoTex) { 
				m_pAlbedoTex->Release();
			}
			else {
				SetAlbedoScale(CVector4::One());//�A���x�h�X�P�[����������
			}
			m_pAlbedoTex = tex;
			if (m_pAlbedoTex) {
				m_pAlbedoTex->AddRef();
			}
		}
		//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
		void SetDefaultAlbedoTexture();

		//�m�[�}���}�b�v���擾
		ID3D11ShaderResourceView* GetNormalTexture()const {
			return m_pNormalTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfNormalTexture()const {
			return &m_pNormalTex;
		}
		//�m�[�}���}�b�v��ݒ�
		void SetNormalTexture(ID3D11ShaderResourceView* tex) {

			if (m_pNormalTex == tex) { return; }//����

			if (m_pNormalTex) {
				m_pNormalTex->Release();
			}
			m_pNormalTex = tex;
			if (m_pNormalTex) {
				m_pNormalTex->AddRef();
			}
		}
		//�m�[�}���}�b�v���f�t�H�ɖ߂�
		void SetDefaultNormalTexture();

		//���C�e�B���O�p�����[�^�}�b�v���擾
		ID3D11ShaderResourceView* GetLightingTexture()const {
			return m_pLightingTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfLightingTexture()const {
			return &m_pLightingTex;
		}
		//���C�e�B���O�p�����[�^�}�b�v��ݒ�
		void SetLightingTexture(ID3D11ShaderResourceView* tex) {

			if (m_pLightingTex == tex) { return; }//����

			if (m_pLightingTex) {
				m_pLightingTex->Release();
			}
			else {
				//������(�����̃p�����[�^�̓e�N�X�`���ɂ�����X�P�[���Ƃ��Ďg��)
				SetEmissive(1.0f);
				SetMetallic(1.0f);
				SetShininess(1.0f);
			}
			m_pLightingTex = tex;
			if (m_pLightingTex) {
				m_pLightingTex->AddRef();
			}
		}
		//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
		void SetDefaultLightingTexture();

		//�g�����X���[�Z���g�}�b�v���擾
		ID3D11ShaderResourceView* GetTranslucentTexture()const {
			return m_pTranslucentTex;
		}
		ID3D11ShaderResourceView* const * GetAddressOfTranslucentTexture()const {
			return &m_pTranslucentTex;
		}
		//�g�����X���[�Z���g�}�b�v��ݒ�
		void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {

			if (m_pTranslucentTex == tex) { return; }//����

			if (m_pTranslucentTex) {
				m_pTranslucentTex->Release();
			}
			else {
				//������(�����̃p�����[�^�̓e�N�X�`���ɂ�����X�P�[���Ƃ��Ďg��)
				SetTranslucent(1.0f);
			}
			m_pTranslucentTex = tex;
			if (m_pTranslucentTex) {
				m_pTranslucentTex->AddRef();
			}
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

		std::wstring m_materialName;  //�}�e���A����
		MaterialParam m_materialParam;//�}�e���A���p�����[�^

		//���_�V�F�[�_
		SKEShaderPtr m_pVSShader;								
		Shader *m_pVSZShader = nullptr; 
		//�s�N�Z���V�F�[�_
		SKEShaderPtr m_pPSShader;		
		Shader *m_pPSZShader = nullptr;

		//�e�N�X�`��
		ID3D11ShaderResourceView* m_pAlbedoTex = nullptr;		
		ID3D11ShaderResourceView* m_pNormalTex = nullptr;
		ID3D11ShaderResourceView* m_pLightingTex = nullptr;
		ID3D11ShaderResourceView* m_pTranslucentTex = nullptr;

		//�ݒ�
		bool m_enableMotionBlur = true;
		bool m_isUseTexZShader = false;
	};

}