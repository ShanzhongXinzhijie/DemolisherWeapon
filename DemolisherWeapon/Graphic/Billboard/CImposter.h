#pragma once

namespace DemolisherWeapon {

	/// <summary>
	/// �C���|�X�^�[�̃C���X�^���V���O�`��ɂ�����e�N�X�`���C���f�b�N�X�������N���X
	/// </summary>
	class InstancingImposterIndex : public GameObj::InstancingModel::IInstancesData {
	public:
		void PreDrawUpdate()override;
		void PostLoopPostUpdate()override;

	public:
		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		/// <param name="instancingMaxNum">�C���X�^���X�ő吔</param>
		InstancingImposterIndex(int instancingMaxNum);
	
		/// <summary>
		/// ���̃t���[���ɕ`�悷��C���X�^���X�̒ǉ�
		/// </summary>
		/// <param name="x">���C���f�b�N�X</param>
		/// <param name="y">�c�C���f�b�N�X</param>
		void AddDrawInstance(int x, int y);

	private:
		std::unique_ptr<int[][2]>							m_instancingIndex;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_indexSB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_indexSRV;
		int m_instanceNum = 0;
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
		float m_imposterMaxSize;//���f���̑傫��
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
		/// <param name="instancingNum">�C���X�^���V���O�`�搔</param>
		void Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum, int instancingNum = 1);
		
		//���W�E��]�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			m_pos = pos;
		}
		/*void SetRot(const CQuaternion& rot) {
			m_billboard.SetRot(rot);
		}*/
		void SetScale(float scale) {
			m_billboard.SetScale(scale*(m_texture->GetModelSize()*2.0f));
			m_scale = scale;
		}
		/*void SetPRS(const CVector3& pos, const CQuaternion& rot, float scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}*/
		//���W�E��]�E�g��̎擾
		const CVector3& GetPos() const {
			return m_pos;
		}
		/*const CQuaternion& GetRot() const {
			return m_billboard.GetRot();
		}*/
		float GetScale() const {
			return m_scale;
		}
		/*void GetPRS(CVector3* pos, CQuaternion* rot, CVector3* scale) const {
			*pos = GetPos();
			*rot = GetRot();
			*scale = GetScale();
		}*/

		//�`�悷�邩�ݒ�
		void SetIsDraw(bool flag) {
			m_billboard.SetIsDraw(flag);
		}
		
	private:
		bool m_isInit = false;
		//�e�N�X�`��
		ImposterTexRender* m_texture = nullptr;
		//���X�^���C�U�[�X�e�[�g
		//�f�v�X�o�C�A�X�p...
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_depthRSCw;
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_depthRSCCw;
		//�r���{�[�h
		CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		CVector3 m_pos;
		float m_scale = 1.0f;
		//�C���X�^���V���O�p
		InstancingImposterIndex* m_instancingIndex = nullptr;
	};
}
}