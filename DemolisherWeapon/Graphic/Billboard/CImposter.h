#pragma once

namespace DemolisherWeapon {

	class ImposterTexRender;

	/// <summary>
	/// �C���|�X�^�[�̃C���X�^���V���O�`��ɂ�����p�����[�^�������N���X
	/// </summary>
	class InstancingImposterParamManager : public GameObj::InstancingModel::IInstancesData {
	private:
		//�Ċm��
		void Reset(int instancingMaxNum);
		//�p�x��ǉ�
		void AddRotY(int instanceIndex, float rad);

	public:
		void PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask)override;
		void AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param)override;
		void SetInstanceMax(int instanceMax)override;

	public:
		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		/// <param name="instancingMaxNum">�C���X�^���X�ő吔</param>
		/// <param name="tex">�C���|�X�^�[�e�N�X�`��</param>
		InstancingImposterParamManager(int instancingMaxNum, ImposterTexRender* tex);		

	private:
		ImposterTexRender* m_texture = nullptr;

		//�p�����[�^
		std::unique_ptr<CVector2[]>							m_params, m_paramsCache;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_paramsSB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_paramsSRV;

		int m_instanceMax = 0;
	};

	/// <summary>
	/// �C���|�X�^�[�̃e�N�X�`�������N���X
	/// </summary>
	class ImposterTexRender {
	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="model">3D���f��</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕�����</param>
		void Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum);

		/// <summary>
		/// ���f���̃T�C�Y���擾
		/// </summary>
		/// <returns>���f���̃T�C�Y</returns>
		float GetModelSize()const {
			return m_imposterMaxSize;
		}

		/// <summary>
		/// ���f���̃J�����ւ̕����̃T�C�Y�𓾂�
		/// </summary>
		/// <param name="x">���C���f�b�N�X</param>
		/// <param name="y">�c�C���f�b�N�X</param>
		/// <returns>���f���̃J�����ւ̕����̃T�C�Y</returns>
		float GetDirectionOfCameraSize(int x, int y)const {
			return m_toCamDirSize[(m_partNumY-1+y)*m_partNumX + x];
		}

		/// <summary>
		/// ���f���̃J�����ւ̕����̃T�C�YSRV�𒸓_�V�F�[�_�ɐݒ�
		/// </summary>
		void VSSetSizeToCameraSRV();

		/// <summary>
		/// �e�N�X�`���������̎擾
		/// </summary>
		/// <returns>�e�N�X�`��������</returns>
		UINT GetPartNumX()const { return m_partNumX; }
		UINT GetPartNumY()const { return m_partNumY; }

		/// <summary>
		/// �e�N�X�`���̎��
		/// </summary>
		enum EnGBuffer {
			enGBufferAlbedo,
			enGBufferNormal,
			enGBufferLightParam,
			enGBufferTranslucent,
			enGBufferNum,
		};

		/// <summary>
		/// SRV�̎擾
		/// </summary>
		/// <param name="type">�擾����e�N�X�`���̎��</param>
		/// <returns>SRV</returns>
		ID3D11ShaderResourceView* GetSRV(EnGBuffer type)const {
			return m_GBufferSRV[type].Get();
		}

	private:
		//�C���|�X�^�e�N�X�`���̍쐬
		void Render(SkinModel& model);

	private:
		//�e�e�N�X�`��		
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_GBufferTex[enGBufferNum];	//GBuffer�e�N�X�`��
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_GBufferView[enGBufferNum];//GBuffer�r���[
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_GBufferSRV[enGBufferNum];	//GBufferSRV
		
		UINT m_gbufferSizeX = 0, m_gbufferSizeY = 0;//�e�N�X�`���T�C�Y
		UINT m_partNumX = 0, m_partNumY = 0;		//�e�N�X�`��������
		float m_imposterMaxSize;//���f���̑傫��
		CVector3 m_boundingBoxMaxSize, m_boundingBoxMinSize;

		//�������ꂽ�e�e�N�X�`���̃��f���̃J���������̑傫��
		std::unique_ptr<float[]>							m_toCamDirSize;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_toCamDirSizeSB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_toCamDirSizeSRV;
	};

	/// <summary>
	/// ImposterTexRender���Ǘ�����N���X
	/// Flyweight�p�^�[����FlyweightFactory�N���X
	/// </summary>
	class ImposterTexBank {
	//�V���O���g��
	private:
		ImposterTexBank() = default;
		~ImposterTexBank() { Release(); }
	public:
		ImposterTexBank(const ImposterTexBank&) = delete;
		ImposterTexBank& operator=(const ImposterTexBank&) = delete;
		ImposterTexBank(ImposterTexBank&&) = delete;
		ImposterTexBank& operator=(ImposterTexBank&&) = delete;
	public:
		//�C���X�^���X���擾
		static ImposterTexBank& GetInstance()
		{
			if (instance == nullptr) {
				instance = new ImposterTexBank;
			}
			return *instance;
		}
	private:
		static ImposterTexBank* instance;
		//�C���X�^���X�̍폜
		static void DeleteInstance() {
			if (instance) {
				delete instance; instance = nullptr;
			}
		}

	public:
		/// <summary>
		/// �C���|�X�^�[�e�N�X�`���̃��[�h
		/// </summary>
		/// <param name="identifier">���ʖ�</param>
		/// <param name="model">3d���f��</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕����� ����𐄏�?</param>
		/// <returns>�C���|�X�^�[�e�N�X�`��</returns>
		ImposterTexRender* Load(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum);
		
		/// <summary>
		/// ���[�h�ς݂̃e�N�X�`�����擾
		/// </summary>
		/// <param name="identifier">���ʖ�</param>
		ImposterTexRender* Get(const wchar_t* identifier);

		/// <summary>
		/// �e�N�X�`���̊J��
		/// </summary>
		void Release();

	private:
		std::unordered_map<int, ImposterTexRender*> m_impTexMap;
	};
	
	class CImposter
	{
	private:
		/// <summary>
		/// �������Ŏg�p����֐�
		/// </summary>
		void InnerInit(const wchar_t* identifier, int instancingNum);

	public:
		/// <summary>
		/// ������(���\�[�X�V�K�쐬)
		/// </summary>
		/// <param name="identifier">�ݒ肷�鎯�ʖ�</param>
		/// <param name="model">�e�N�X�`���ɂ���3D���f��</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕����� ����𐄏�?</param>
		/// <param name="instancingNum">�C���X�^���V���O�`�搔</param>
		bool Init(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, int instancingNum);
		/// <summary>
		/// ������(�ǂݍ��ݍς݂̃��\�[�X���g�p)
		/// </summary>
		/// <param name="identifier">���\�[�X�̎��ʖ�</param>
		/// <param name="instancingNum">�C���X�^���V���O�`�搔</param>
		bool Init(const wchar_t* identifier, int instancingNum);

		//���W�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			m_pos = pos;
			m_billboard.SetPos(m_pos);
		}
		void SetScale(float scale) {
			m_scale = scale;
			m_billboard.SetScale(m_scale * m_texture->GetModelSize()*2.0f);
			m_billboard.GetModel().GetSkinModel().SetImposterParameter(m_scale, m_rotYrad);
		}
		//Y����]�p�x
		void SetRotY(float rad) {
			m_rotYrad = rad; 
			if (m_rotYrad < -CMath::PI2) { m_rotYrad += CMath::PI2; }
			if (m_rotYrad > CMath::PI2) { m_rotYrad -= CMath::PI2; }
			m_billboard.GetModel().GetSkinModel().SetImposterParameter(m_scale, m_rotYrad);			
		}

		//���W�E�g��̎擾
		const CVector3& GetPos() const {
			return m_pos;
		}
		float GetScale() const {
			return m_scale;
		}

		//�`�悷�邩�ݒ�
		void SetIsDraw(bool flag) {
			m_billboard.SetIsDraw(flag);
		}
		//�V���h�E�}�b�v�ɕ`�悷�邩�ݒ�
		void SetIsShadowCaster(bool flag) {
			m_billboard.GetModel().SetIsShadowCaster(flag);
		}

	private:
		bool m_isInit = false;
		//�e�N�X�`��
		ImposterTexRender* m_texture = nullptr;
		//�r���{�[�h
		CBillboard m_billboard;
		SkinModelEffectShader m_imposterPS;
		Shader m_zShader, m_vsShader, m_vsZShader;

		CVector3 m_pos;
		float m_scale = 1.0f, m_rotYrad = 0.0f;
	};
	
}