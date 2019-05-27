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
		//std::vector<std::vector<CVector3>> m_fronts, m_ups;//�������ꂽ�e�e�N�X�`���̑O�����Ə����

		//�e�N�X�`�������p
		SkinModelEffectShader m_imposterPS;
		float m_imposterMaxSize;//���f���̑傫��
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_depthStencilTex;			//�f�v�X�X�e���V���e�N�X�`��
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_depthStencilView;			//�f�v�X�X�e���V���r���[
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

namespace GameObj {

	class CImposter : public IGameObject
	{
	public:
		void PostLoopUpdate()override;

	public:
		/// <summary>
		/// ������
		/// </summary>
		/// <param name="filepath">3D���f���̃t�@�C���p�X</param>
		/// <param name="resolution">�C���|�X�^�[�e�N�X�`���̉𑜓x</param>
		/// <param name="partNum">�C���|�X�^�[�e�N�X�`���̕����� ����𐄏�?</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum);
		
		//���W�E��]�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			m_billboard.SetPos(pos);
		}
		void SetRot(const CQuaternion& rot) {
			m_billboard.SetRot(rot);
		}
		void SetScale(const CVector3& scale) {
			m_billboard.SetScale(scale*(m_texture->GetModelSize()*2.0f));
			m_scale = scale;
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//���W�E��]�E�g��̎擾
		const CVector3& GetPos() const {
			return m_billboard.GetPos();
		}
		const CQuaternion& GetRot() const {
			return m_billboard.GetRot();
		}
		const CVector3& GetScale() const {
			return m_scale;
		}
		void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const {
			*pos = GetPos();
			*rot = GetRot();
			*scale = GetScale();
		}

		//�`�悷�邩�ݒ�
		void SetIsDraw(bool flag) {
			m_billboard.GetModel().SetIsDraw(flag);
		}
		
	private:
		bool m_isInit = false;
		//�e�N�X�`��
		ImposterTexRender* m_texture = nullptr;
		//�r���{�[�h
		CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		CVector3 m_scale;
	};
}
}