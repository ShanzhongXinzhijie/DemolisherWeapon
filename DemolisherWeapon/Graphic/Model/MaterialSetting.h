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
	/// �}�e���A���p�����[�^�[
	/// �萔�o�b�t�@�@[model.fx:MaterialCb]
	/// </summary>
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

	/// <summary>
	/// �}�e���A���ݒ�
	/// </summary>
	class MaterialSetting
	{
	public:
		MaterialSetting() = default;

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="modeleffect">�e���f���G�t�F�N�g</param>
		//void Init(ModelEffect* modeleffect);
		void Init(MaterialData* materialData);

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

		/// <summary>
		/// TriPlanarMapping�V�F�[�_��ݒ�
		/// </summary>
		/// <param name="isYOnly">�}�b�s���O���c�����Ɍ��肷�邩</param>
		void SetTriPlanarMappingPS(bool isYOnly = false);
		//TriPlanarMapping����UV���W�ւ̃X�P�[����ݒ�
		void SetTriPlanarMappingUVScale(float texScale) {
			m_materialParam.triPlanarMapUVScale = texScale;
		}		

		//�A���x�h�e�N�X�`�����擾
		ID3D11ShaderResourceView* GetAlbedoTexture()const {
			return m_albedo.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfAlbedoTexture()const {
			return m_albedo.textureView.GetAddressOf();
		}
		//�A���x�h�e�N�X�`����ݒ�
		void SetAlbedoTexture(ID3D11ShaderResourceView* tex){
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetAlbedoTexture(texData);
		}
		//�A���x�h�e�N�X�`����ݒ�
		void SetAlbedoTexture(const TextueData& tex) {
			if (m_albedo == tex) { return; }//����

			if (!m_albedo.isLoaded()) {
				SetAlbedoScale(CVector4::One());//�A���x�h�X�P�[����������
			}

			m_albedo = tex;//�R�s�[
		}
		//�A���x�h�e�N�X�`�����f�t�H�ɖ߂�
		void SetDefaultAlbedoTexture();

		//�m�[�}���}�b�v���擾
		ID3D11ShaderResourceView* GetNormalTexture()const {
			return m_normal.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfNormalTexture()const {
			return m_normal.textureView.GetAddressOf();
		}
		//�m�[�}���}�b�v��ݒ�
		void SetNormalTexture(ID3D11ShaderResourceView* tex) {
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetNormalTexture(texData);
		}
		//�m�[�}���}�b�v��ݒ�
		void SetNormalTexture(const TextueData& tex) {
			if (m_normal == tex) { return; }//����
			m_normal = tex;//�R�s�[
		}
		//�m�[�}���}�b�v���f�t�H�ɖ߂�
		void SetDefaultNormalTexture();

		//���C�e�B���O�p�����[�^�}�b�v���擾
		ID3D11ShaderResourceView* GetLightingTexture()const {
			return m_lighting.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfLightingTexture()const {
			return m_lighting.textureView.GetAddressOf();
		}
		//���C�e�B���O�p�����[�^�}�b�v��ݒ�
		void SetLightingTexture(ID3D11ShaderResourceView* tex) {
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetLightingTexture(texData);
		}
		//���C�e�B���O�p�����[�^�}�b�v��ݒ�
		void SetLightingTexture(const TextueData& tex) {
			if (m_lighting == tex) { return; }//����

			if (!m_lighting.isLoaded()) {
				//������(�����̃p�����[�^�̓e�N�X�`���ɂ�����X�P�[���Ƃ��Ďg��)
				SetEmissive(1.0f);
				SetMetallic(1.0f);
				SetShininess(1.0f);
			}

			m_lighting = tex;//�R�s�[
		}
		//���C�e�B���O�p�����[�^�}�b�v���f�t�H�ɖ߂�
		void SetDefaultLightingTexture();

		//�g�����X���[�Z���g�}�b�v���擾
		ID3D11ShaderResourceView* GetTranslucentTexture()const {
			return m_translucent.textureView.Get();
		}
		ID3D11ShaderResourceView* const * GetAddressOfTranslucentTexture()const {
			return m_translucent.textureView.GetAddressOf();
		}
		//�g�����X���[�Z���g�}�b�v��ݒ�
		void SetTranslucentTexture(ID3D11ShaderResourceView* tex) {
			TextueData texData;
			if (tex) {
				tex->AddRef();
				texData.textureView.Attach(tex);
			}
			SetTranslucentTexture(texData);
		}
		//�g�����X���[�Z���g�}�b�v��ݒ�
		void SetTranslucentTexture(const TextueData& tex) {
			if (m_translucent == tex) { return; }//����

			if (!m_translucent.isLoaded()) {
				//������(�����̃p�����[�^�̓e�N�X�`���ɂ�����X�P�[���Ƃ��Ďg��)
				SetTranslucent(1.0f);
			}

			m_translucent = tex;//�R�s�[
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

		//�X�L�����f�����擾
		bool GetIsSkining()const;

		//ModelEffect* GetModelEffect() {
		//	return m_isInit;
		//}
		
	private:
		MaterialData* m_isInit = nullptr;//�e

		std::wstring m_materialName;  //�}�e���A����
		MaterialParam m_materialParam;//�}�e���A���p�����[�^

		//���_�V�F�[�_
		SKEShaderPtr m_pVSShader;								
		Shader *m_pVSZShader = nullptr; 
		//�s�N�Z���V�F�[�_
		SKEShaderPtr m_pPSShader;		
		Shader *m_pPSZShader = nullptr;

		//�e�N�X�`��
		TextueData m_albedo;
		TextueData m_normal;
		TextueData m_lighting;
		TextueData m_translucent;

		//�ݒ�
		bool m_enableMotionBlur = true;
		bool m_isUseTexZShader = false;
	};

	/// <summary>
	/// �}�e���A���f�[�^
	/// </summary>
	class MaterialData {
	public:
		MaterialData() = default;

		//������
		void Init(bool isSkining, std::wstring_view name);
		void InitAlbedoTexture(std::wstring_view path);
		void InitAlbedoColor(const CVector3& rgb);
		void InitNormalTexture(std::wstring_view path);
		void InitLightingTexture(std::wstring_view path);

		//�f�t�H���g�̃V�F�[�_���擾
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
		//TriPlanarMapping�p�̃V�F�[�_���擾
		SkinModelEffectShader* GetTriPlanarMappingPS(bool isYOnly) {
			return isYOnly ? &m_psTriPlanarMapShaderYOnly : &m_psTriPlanarMapShader;
		}

		//�f�t�H���g�̃A���x�h�e�N�X�`�����擾
		TextueData& GetDefaultAlbedoTexture() {
			return m_albedo;
		}
		//�f�t�H���g�̃m�[�}���}�b�v���擾
		TextueData& GetDefaultNormalTexture() {
			return m_normal;
		}
		//�f�t�H���g�̃��C�e�B���O�p�����[�^�}�b�v���擾
		TextueData& GetDefaultLightingTexture() {
			return m_lighting;
		}

		//�萔�o�b�t�@�̎擾(DX11)
		auto& GetConstantBufferDX11() {
			return m_materialParamCBDX11;
		}

		//�X�L�����f�����擾
		bool GetIsSkining()const {
			return m_isSkining;
		}

		//�f�t�H���g�}�e���A���ݒ�̎擾
		MaterialSetting& GetDefaultMaterialSetting() {
			return m_defaultMaterialSetting;
		}
		//�g�p���̃}�e���A���ݒ�̎擾
		MaterialSetting& GetUsingMaterialSetting() {
			return *m_ptrUseMaterialSetting;
		}

		//�g���}�e���A���ݒ�
		void SetUseMaterialSetting(MaterialSetting& matset) {
			m_ptrUseMaterialSetting = &matset;
		}
		//���f���f�[�^�f�t�H���g�̃}�e���A���ݒ���g�p
		void SetDefaultMaterialSetting() {
			SetUseMaterialSetting(m_defaultMaterialSetting);
		}

	private:
		//�f�t�H���g�o�[�e�b�N�X�V�F�[�_
		SkinModelEffectShader m_vsDefaultShader;
		Shader m_vsZShader;//Z�l�o�͗p

		//�f�t�H���g�s�N�Z���V�F�[�_
		SkinModelEffectShader m_psDefaultShader;
		Shader m_psZShader[2];//Z�l�o�͗p
		SkinModelEffectShader m_psTriPlanarMapShader, m_psTriPlanarMapShaderYOnly;//TriPlanarMapping�p�̃V�F�[�_

		//�X�L�����f�����H
		bool m_isSkining;

		//�e�N�X�`��
		TextueData m_albedo;
		TextueData m_normal;
		TextueData m_lighting;

		//�}�e���A���ݒ�
		MaterialSetting* m_ptrUseMaterialSetting = nullptr;	//�g�p����}�e���A���ݒ�
		MaterialSetting m_defaultMaterialSetting;	//�}�e���A���ݒ�(�f�t�H���g)

		//�}�e���A���p�����[�^�p�̒萔�o�b�t�@
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialParamCBDX11;
		std::unique_ptr<ConstantBufferDx12<MaterialParam>> m_materialParamCBDX12;

		//�t�����h
		friend class ModelEffect;
	};
}