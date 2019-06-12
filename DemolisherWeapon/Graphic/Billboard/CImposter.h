#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// �C���|�X�^�[�̃e�N�X�`�������N���X
	/// </summary>
	class ImposterTexRender {
	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="filepath">3D���f���̃t�@�C���p�X</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕�����</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum);

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
		/// <param name="filepath">3D���f���̃t�@�C���p�X</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕�����</param>
		/// <returns>�C���|�X�^�[�e�N�X�`��</returns>
		ImposterTexRender* Load(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum);

		/// <summary>
		/// �e�N�X�`���̊J��
		/// </summary>
		void Release();

	private:
		std::unordered_map<int, ImposterTexRender*> m_impTexMap;
	};
	
	class CImposter
	{
	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="filepath">3D���f���̃t�@�C���p�X</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕����� ����𐄏�?</param>
		/// <param name="instancingNum">�C���X�^���V���O�`�搔</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum, int instancingNum = 1);
		
		//���W�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			m_pos = pos;
			m_billboard.SetPos(m_pos);
		}
		void SetScale(float scale) {
			m_scale = scale;
			m_billboard.SetScale(m_scale * m_texture->GetModelSize()*2.0f);
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
		
	private:
		bool m_isInit = false;
		//�e�N�X�`��
		ImposterTexRender* m_texture = nullptr;
		//�r���{�[�h
		CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		Shader m_zShader, m_vsShader, m_vsZShader;
		CVector3 m_pos;
		float m_scale = 1.0f;
	};
	
}