#pragma once

namespace DemolisherWeapon {

namespace GameObj {

	//�C���X�^���V���O�p���f��
	class InstancingModel : public IQSGameObject {
	public:
		InstancingModel() {};
		~InstancingModel() { Release(); };

		void PostLoopPostUpdate()override;

	public:
		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		/// <param name="instanceMax">�ő�C���X�^���X��</param>
		/// <param name="filePath">���f���̃t�@�C���p�X</param>
		/// <param name="animationClip">�A�j���[�V�����N���b�v</param>
		/// <param name="fbxUpAxis">�������</param>
		/// <param name="fbxCoordinate">���W�n</param>
		InstancingModel(int instanceMax,
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		) {
			Init(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
		}

		void Release() {
			//�C���X�^���V���O�p���\�[�X�̊J��
			m_instanceMax = 0;
			m_instanceNum = 0;
			m_instancingWorldMatrix.reset();
			if (m_worldMatrixSB) { m_worldMatrixSB->Release(); m_worldMatrixSB = nullptr; }
			if (m_worldMatrixSRV) { m_worldMatrixSRV->Release(); m_worldMatrixSRV = nullptr; }
			m_instancingWorldMatrixOld.reset();
			if (m_worldMatrixSBOld) { m_worldMatrixSBOld->Release(); m_worldMatrixSBOld = nullptr; }
			if (m_worldMatrixSRVOld) { m_worldMatrixSRVOld->Release(); m_worldMatrixSRVOld = nullptr; }
		}

		//������
		void Init(int instanceMax,
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		);

		//�ő�C���X�^���X���̐ݒ�
		void SetInstanceMax(int instanceMax);
		int  GetInstanceMax()const { return m_instanceMax; }

		//���f���̎擾
		GameObj::CSkinModelRender& GetModelRender() { return m_model; }

		//�`��O�ɂ�鏈����ݒ�
		void SetPreDrawFunction(std::function<void()> func) {
			m_preDrawFunc = func;
		}

		//���̃t���[���ɕ`�悷��C���X�^���X�̒ǉ�
		void AddDrawInstance(const CMatrix* woridMatrix, const CMatrix* woridMatrixOld = nullptr) {
			if (m_instanceNum + 1 >= m_instanceMax) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "�yInstancingModel�z�C���X�^���X�̍ő吔�ɒB���Ă��܂��I\n���f����:%ls\n�C���X�^���X�ő吔:%d\n", m_model.GetSkinModel().GetModelName(), m_instanceMax);
				OutputDebugStringA(message);
#endif				
				return;
			}

			if (!woridMatrixOld) { woridMatrixOld = woridMatrix; }
			m_instancingWorldMatrix[m_instanceNum] = *woridMatrix;
			m_instancingWorldMatrixOld[m_instanceNum] = *woridMatrixOld;
			m_instanceNum++;
		}

		//�C���X�^���X���Ƃ̃f�[�^�������p�̃C���^�[�t�F�C�X�N���X 
		class IInstancesData {
		public:
			virtual ~IInstancesData() {};
		public:
			//�`��O�Ɏ��s���鏈��
			//���SRV�̐ݒ������
			virtual void PreDrawUpdate() = 0;
			//PostLoopPostUpdate�Ŏ��s���鏈��
			//��ɃX�g���N�`���[�o�b�t�@�̍X�V������
			virtual void PostLoopPostUpdate() = 0;
			//CInstancingModelRender��AddDrawInstance�Ŏ��s���鏈��
			//��ɃC���X�^���X���Ƃ̃f�[�^��ǉ�����
			//virtual void AddDrawInstance() {}// = 0;
		};
		/// <summary>
		/// IInstanceData���Z�b�g
		/// </summary>
		/// <param name="IID">IInstanceData</param>
		void SetIInstanceData(std::unique_ptr<IInstancesData>&& IID) {
			m_instanceData = std::move(IID);
		}
		/// <summary>
		/// �ݒ肳��Ă���IInstanceData���擾
		/// </summary>
		/// <returns>�ݒ肳��Ă���IInstanceData</returns>
		IInstancesData* GetIInstanceData()const {
			return m_instanceData.get();
		}
		/// <summary>
		/// IInstanceData��AddDrawInstance�����s����
		/// </summary>
		/*void IInstanceData_AddDrawInstance(){
			if (m_instanceData) {
				m_instanceData->AddDrawInstance();
			}
		}*/

	private:
		int m_instanceNum = 0;
		int m_instanceMax = 0;

		GameObj::CSkinModelRender m_model;
		AnimationClip m_animationClip;

		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrix;
		ID3D11Buffer*				m_worldMatrixSB = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRV = nullptr;

		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrixOld;
		ID3D11Buffer*				m_worldMatrixSBOld = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRVOld = nullptr;

		std::function<void()> m_preDrawFunc = nullptr;

		std::unique_ptr<IInstancesData> m_instanceData;

		Shader m_vsShader, m_vsZShader;
		Shader m_vsSkinShader, m_vsZSkinShader;
	};

}
	//�C���X�^���V���O�p���f���̃}�l�[�W���[
	class InstancingModelManager {
	public:
		~InstancingModelManager() {
			Release();
		}

		void Release() {
			//�}�b�v���̂��ׂĂ�InstancingModel�����
			for(auto& p : m_instancingModelMap){
				delete p.second;
			}
		}

		//�w��̃��f�����폜
		void Delete(const wchar_t* filePath, const AnimationClip* animationClip = nullptr, const wchar_t* identifier = nullptr);

		GameObj::InstancingModel* Load(
			int instanceMax,										//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t* filePath,
			const AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//�V�K�ǂݍ��ݎ��̂ݎg�p
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t* identifier = nullptr
		);
	private:
		typedef std::tuple<std::size_t, std::size_t, std::size_t> key_t;

		struct key_hash 
		{
			std::size_t operator()(const key_t& k) const
			{
				return Util::HashCombine(Util::HashCombine(std::get<0>(k), std::get<1>(k)), std::get<2>(k));
			}
		};

		std::unordered_map<key_t, GameObj::InstancingModel*, key_hash> m_instancingModelMap;
	};

namespace GameObj {

	//�C���X�^���V���O�p���f���̃����_���[
	class CInstancingModelRender : public IQSGameObject
	{
	public:
		//������
		void Init(int instanceMax,									//�ő�`�搔
			const wchar_t* filePath,								//���f���̃t�@�C���p�X
			const AnimationClip* animationClips = nullptr,			//�A�j���[�V�����N���b�v�̔z��
			int numAnimationClips = 0,								//�A�j���[�V�����N���b�v�̐�
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//���f���̏����	//�V�K�ǂݍ��ݎ��̂ݎg�p
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//���W�n			//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t** identifiers = nullptr					//���ʎq�A���ꂪ�Ⴄ�ƕʃ��f���Ƃ��Ĉ����� //numAnimationClips�Ɠ����K�v
		) {
			//�A�j���[�V�����̐��������f�����[�h
			m_model.clear();
			for (int i = 0; i < max(numAnimationClips,1); i++) {
				const wchar_t* identifier = nullptr; if (identifiers) { identifier = identifiers[i]; }
				m_model.emplace_back(m_s_instancingModelManager.Load(instanceMax, filePath, &animationClips[i], fbxUpAxis, fbxCoordinate, identifier));
				if (m_model.back()->GetInstanceMax() < instanceMax) {
					m_model.back()->SetInstanceMax(instanceMax);
				}
			}
			m_playingAnimNum = 0;

			m_isInit = true;
		}
		
		void PostLoopUpdate()override final {
			if (!m_isInit) { return; }
			if (!m_isDraw) { m_isFirstWorldMatRef = true; return; }

			//���[���h�s������߂�(�o�C�A�X�܂�)
			m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcWorldMatrix( m_pos, m_rot, m_scale, m_worldMatrix);

			//�ŏ��̃��[���h���W�X�V�Ȃ�...
			if (m_isFirstWorldMatRef) {
				m_isFirstWorldMatRef = false;
				//�����W�̍X�V
				m_worldMatrixOld = m_worldMatrix;
			}

			//�C���X�^���V���O���f���ɑ���
			if (m_isDraw) {
				m_model[m_playingAnimNum]->AddDrawInstance(&m_worldMatrix, &m_worldMatrixOld);
				//m_model[m_playingAnimNum]->IInstanceData_AddDrawInstance();
			}
			m_worldMatrixOld = m_worldMatrix;
		}

		//���W�Ƃ��ݒ�
		void SetPos(const CVector3& pos) {
			m_pos = pos;
		}
		void SetRot(const CQuaternion& rot) {
			m_rot = rot;
		}
		void SetScale(const CVector3& scale) {
			m_scale = scale;
		}
		void SetPRS(const CVector3& pos, const CQuaternion& rot, const CVector3& scale) {
			SetPos(pos);
			SetRot(rot);
			SetScale(scale);
		}
		//���W�Ƃ��擾
		const CVector3& GetPos()const {
			return m_pos;
		}
		const CQuaternion& GetRot()const {
			return m_rot;
		}
		const CVector3& GetScale()const {
			return m_scale;
		}

		//�Đ��A�j���[�V�����̕ύX
		void ChangeAnim(int animNum) {
			if (animNum >= m_model.size()) { return; }
			m_playingAnimNum = animNum;
		}
		int GetPlayAnimNum()const {
			return m_playingAnimNum;
		}

		//�`�悷�邩�ݒ�
		void SetIsDraw(bool enable) {
			m_isDraw = enable;
		}
		bool GetIsDraw() const{
			return m_isDraw;
		}

		//���f���̎擾
		InstancingModel* GetInstancingModel(int num) { return m_model[num]; }
		InstancingModel* GetInstancingModel() { return GetInstancingModel(m_playingAnimNum); }

	private:
		bool m_isInit = false;
		bool m_isDraw = true;

		std::vector<GameObj::InstancingModel*> m_model;
		int m_playingAnimNum = 0;

		bool m_isFirstWorldMatRef = true;
		CVector3 m_pos;
		CQuaternion m_rot;
		CVector3 m_scale = CVector3::One();
		CMatrix m_worldMatrix, m_worldMatrixOld;

	public:
		static InstancingModelManager& GetInstancingModelManager() { return m_s_instancingModelManager; }
	private:
		static InstancingModelManager m_s_instancingModelManager;
	};

}
}