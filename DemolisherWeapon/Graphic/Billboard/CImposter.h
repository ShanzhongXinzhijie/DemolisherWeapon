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
		void AddDrawInstance(int x, int y, const CVector3& pos, float scale);
		
		//�C���f�b�N�X�̎擾
		const std::unique_ptr<int[][2]>& GetIndexs()const {
			return m_instancingIndex;
		}
		//�C���f�b�N�X�̐ݒ�ƍX�V
		void SetUpdateDrawIndex(const int index[][2]) {
			//�ݒ�
			for (int i = 0; i < m_instanceDrawNum; i++) {
				m_instancingIndex[i][0] = index[i][0];
				m_instancingIndex[i][1] = index[i][1];
			}
			//StructuredBuffer���X�V
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
				m_indexSB.Get(), 0, NULL, m_instancingIndex.get(), 0, 0
			);
		}

		//���W�擾
		const std::unique_ptr<CVector3[]>& GetPoses()const {
			return m_instancingPos;
		}
		//�X�P�[���̎擾
		const std::unique_ptr<float[]>& GetScales()const {
			return m_instancingScale;
		}

	private:
		std::unique_ptr<int[][2]>							m_instancingIndex;
		std::unique_ptr<CVector3[]>							m_instancingPos;
		std::unique_ptr<float[]>							m_instancingScale;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_indexSB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_indexSRV;
		int m_instanceNum = 0, m_instanceDrawNum = 0;
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
		/// ���f���̃J�����ւ̕����̃T�C�Y�𓾂�
		/// </summary>
		/// <param name="x">���C���f�b�N�X</param>
		/// <param name="y">�c�C���f�b�N�X</param>
		/// <returns>���f���̃J�����ւ̕����̃T�C�Y</returns>
		float GetDirectionOfCameraSize(int x, int y)const {
			return m_toCamDirSize[m_partNumY-1+y][x];
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
		float m_imposterMaxSize;//���f���̑傫��
		CVector3 m_boundingBoxMaxSize, m_boundingBoxMinSize;
		std::vector<std::vector<float>> m_toCamDirSize;//�������ꂽ�e�e�N�X�`���̃��f���̃J���������̑傫��
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
		
		//���[���h�s��ƃC���f�b�N�X�̌v�Z
		static void CalcWorldMatrixAndIndex(const SkinModel& model, const ImposterTexRender& texture, const CVector3& pos, float scale, CVector3& position_return, CQuaternion& rotation_return, float& scale_return, int& index_x_return, int& index_y_return);

		//���W�E�g��̐ݒ�
		void SetPos(const CVector3& pos) {
			m_pos = pos;
		}
		void SetScale(float scale) {
			m_scale = scale;
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
		//���X�^���C�U�[�X�e�[�g
		//�f�v�X�o�C�A�X�p...
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_depthRSCw;
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_depthRSCCw;
		//�r���{�[�h
		CBillboard m_billboard;
		SkinModelEffectShader m_billboardPS;
		Shader m_zShader;
		CVector3 m_pos;
		float m_scale = 1.0f;
		//�C���X�^���V���O�p
		InstancingImposterIndex* m_instancingIndex = nullptr;
	};
}
	/// <summary>
	/// �C���|�X�^�[�̃V���h�E�}�b�v�`�掞�Ɏ��s���鏈��
	/// </summary>
	class ShodowWorldMatrixCalcerImposter : public ShadowMapRender::IPrePost {
	public:
		ShodowWorldMatrixCalcerImposter(GameObj::CImposter* imp, SkinModel* model);
		void PreDraw()override;
		void PreModelDraw()override;
		void PostDraw()override;
	private:
		int m_x = 0, m_y = 0;
		CMatrix	m_worldMatrix;
		GameObj::CImposter* m_ptrImposter = nullptr;
		SkinModel* m_ptrModel = nullptr;
	};
	//�C���X�^���V���O�p
	class ShodowWorldMatrixCalcerInstancingImposter : public ShadowMapRender::IPrePost {
	public:
		ShodowWorldMatrixCalcerInstancingImposter(ImposterTexRender* tex, GameObj::InstancingModel* model, InstancingImposterIndex* index);
		void PreDraw()override;
		void PreModelDraw()override;
		void PostDraw()override;
	private:
		int m_instancesNum = 0;
		std::unique_ptr<int[][2]> m_index, m_indexNew;
		std::unique_ptr<CMatrix[]>	m_worldMatrix, m_worldMatrixNew;
		ImposterTexRender* m_ptrTexture = nullptr;
		GameObj::InstancingModel* m_ptrModel = nullptr;
		InstancingImposterIndex* m_ptrIndex = nullptr;
	};
}