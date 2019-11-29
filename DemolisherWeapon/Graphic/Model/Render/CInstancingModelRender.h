#pragma once

namespace DemolisherWeapon {

	class InstanceWatcher;

namespace GameObj {

	//�C���X�^���V���O�p���f��
	class InstancingModel : public IQSGameObject {
	public:
		InstancingModel() = default;
		~InstancingModel() { Release(); };
						
		void PreLoopUpdate()override;

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
			m_instanceIndex = 0; m_instanceDrawNum = 0;

			m_instancingWorldMatrix.reset();
			if (m_worldMatrixSB) { m_worldMatrixSB->Release(); m_worldMatrixSB = nullptr; }
			if (m_worldMatrixSRV) { m_worldMatrixSRV->Release(); m_worldMatrixSRV = nullptr; }
			m_instancingWorldMatrixOld.reset();
			if (m_worldMatrixSBOld) { m_worldMatrixSBOld->Release(); m_worldMatrixSBOld = nullptr; }
			if (m_worldMatrixSRVOld) { m_worldMatrixSRVOld->Release(); m_worldMatrixSRVOld = nullptr; }

			m_drawInstanceMask.reset();
			m_minAABB.reset(); m_maxAABB.reset();
			m_worldMatrixCache.reset();
			m_worldMatrixOldCache.reset();

			m_insWatchers.reset();

			m_instanceData.clear();
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

		//�`��C���X�^���X���̎擾
		//int GetDrawInstanceNum()const { return m_instanceDrawNum; }

		//���f���̎擾
		GameObj::CSkinModelRender& GetModelRender() { return m_model; }

		//�`��O�ɂ�鏈����ݒ�
		void SetPreDrawFunction(std::function<void()> func) {
			m_preDrawFunc = func;
		}

		//���̃t���[���ɕ`�悷��C���X�^���X�̒ǉ�
		void AddDrawInstance(
			const CMatrix& woridMatrix, const CMatrix& woridMatrixOld,
			const CMatrix& SRTMatrix, const CVector3& scale,
			const CVector3& minAABB, const CVector3& maxAABB, 
			void *param_ptr,
			const std::shared_ptr<InstanceWatcher>& watcher
		) {
			if (m_instanceIndex >= m_instanceMax) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "�yInstancingModel�z�C���X�^���X�̍ő吔�ɒB���Ă��܂��I\n���f����:%ls\n�C���X�^���X�ő吔:%d\n", m_model.GetSkinModel().GetModelName(), m_instanceMax);
				OutputDebugStringA(message);
#endif				
				return;
			}

			m_worldMatrixCache[m_instanceIndex] = woridMatrix;
			m_worldMatrixOldCache[m_instanceIndex] = woridMatrixOld;
			m_minAABB[m_instanceIndex] = minAABB;
			m_maxAABB[m_instanceIndex] = maxAABB;

			//IInstanceData�̏������s
			for (auto& IID : m_instanceData) {
				IID.second->AddDrawInstance(m_instanceIndex, SRTMatrix, scale, param_ptr);
			}

			//�Ď��ғo�^
			m_insWatchers[m_instanceIndex] = watcher;

			m_instanceIndex++;
		}

		//������J�����O���s�����ݒ�
		void SetIsFrustumCulling(bool enable) {
			m_isFrustumCull = enable;
		}
		bool GetIsFrustumCulling()const {
			return m_isFrustumCull;
		}

		/// <summary>
		/// �C���X�^���X���Ƃ̃f�[�^�������p�̃C���^�[�t�F�C�X�N���X 
		/// </summary>
		class IInstancesData {
		public:
			virtual ~IInstancesData() {};
		public:
			/// <summary>
			/// �J�����O�O�Ɏ��s���鏈��
			/// </summary>
			/// <param name="instanceIndex">�C���X�^���X�ԍ�</param>
			/// <returns>���̃C���X�^���X��`�悷�邩</returns>
			//virtual bool PreCulling(int instanceIndex) {}

			/// <summary>
			/// �`��O�Ɏ��s���鏈��
			/// ��ɃX�g���N�`���[�o�b�t�@�̍X�V��SRV�̐ݒ������
			/// </summary>
			/// <param name="instanceNum">�C���X�^���X����</param>
			/// <param name="drawInstanceNum">�`�悷��C���X�^���X�̐�(�J�����O����Ȃ��������̐�)</param>
			/// <param name="drawInstanceMask">�J�����O���ꂽ�C���X�^���X�̃}�X�N(false�ɂȂ��Ă����J�����O���ꂽ)</param>
			virtual void PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask) = 0;//{}

			//AddDrawInstance�Ŏ��s���鏈��
			//��ɃC���X�^���X���Ƃ̃f�[�^��ǉ�����
			virtual void AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param) {}

			//SetInstanceMax�Ŏ��s���鏈��
			//�C���X�^���X�ő吔��ݒ�
			virtual void SetInstanceMax(int instanceMax) {}
		};

		/// <summary>
		/// IInstanceData���Z�b�g
		/// </summary>
		/// <param name="identifier">IInstanceData�̖��O</param>
		void AddIInstanceData(const wchar_t* identifier, std::unique_ptr<IInstancesData>&& IID) {
			int index = Util::MakeHash(identifier);
			m_instanceData.emplace(index,std::move(IID));
		}
		/// <summary>
		/// �ݒ肳��Ă���IInstanceData���擾
		/// </summary>
		/// <param name="identifier">IInstanceData�̖��O</param>
		IInstancesData* GetIInstanceData(const wchar_t* identifier)const {
			int index = Util::MakeHash(identifier);
			auto IID = m_instanceData.find(index);
			if (IID == m_instanceData.end()) {
				return nullptr;
			}
			else {
				return IID->second.get();
			}
		}
		/// <summary>
		/// �ݒ肳��Ă���IInstanceData���폜
		/// </summary>
		/// <param name="identifier">IInstanceData�̖��O</param>
		void DeleteIInstanceData(const wchar_t* identifier) {
			int index = Util::MakeHash(identifier);
			auto IID = m_instanceData.find(index);
			if (IID == m_instanceData.end()) {
				//�v�f��������Ȃ�
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "�yInstancingModel::DeleteIInstanceData�z�v�f��������܂���B�v�f��:%ls \n", identifier);
				DW_WARNING_MESSAGE(true, message);
#endif
				return;
			}
			else {
				m_instanceData.erase(index);
			}
		}

	private:
		int m_instanceIndex = 0, m_instanceDrawNum = 0;
		int m_instanceMax = 0;

		//���f��
		GameObj::CSkinModelRender m_model;
		AnimationClip m_animationClip;

		//���[���h�s��
		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrix;
		ID3D11Buffer*				m_worldMatrixSB = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRV = nullptr;
		//�����[���h�s��
		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrixOld;
		ID3D11Buffer*				m_worldMatrixSBOld = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRVOld = nullptr;

		//������J�����O�p
		std::unique_ptr<bool[]>		m_drawInstanceMask;
		std::unique_ptr<CVector3[]> m_minAABB, m_maxAABB;
		std::unique_ptr<CMatrix[]>	m_worldMatrixCache;
		std::unique_ptr<CMatrix[]>	m_worldMatrixOldCache;
		bool m_isFrustumCull = false;//������J�����O���邩?

		//�C���X�^���X�������Ď�����
		std::unique_ptr<std::weak_ptr<InstanceWatcher>[]> m_insWatchers;
		
		//���[�U�[�ݒ�̕`��O����
		std::function<void()> m_preDrawFunc = nullptr;

		//�C���X�^���X���Ƃ̃f�[�^�������p�̃N���X
		std::unordered_map<int,std::unique_ptr<IInstancesData>> m_instanceData;

		//�V�F�[�_
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

		//���f���̃��[�h
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
		CInstancingModelRender();
		~CInstancingModelRender();

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

			//�o�E���f�B���O�{�b�N�X�̏�����
			m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);

			m_isInit = true;
		}
		
		void PostLoopPostUpdate()override final {
			if (!m_isInit) { return; }
			if (!m_isDraw) { m_isFirstWorldMatRef = true; return; }

			//���[���h�s��A�X�V���ĂȂ����
			if (!m_isUpdatedWorldMatrix) {
				if (m_isSetRotOrScale) {
					//���[���h�s������߂�(�o�C�A�X�܂�)
					m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcWorldMatrix(m_pos, m_rot, m_scale, m_worldMatrix, m_SRTMatrix);
					m_isSetRotOrScale = false;
				}
				else {
					//���s�ړ��������X�V
					m_worldMatrix.SetTranslation(m_pos);
					m_SRTMatrix.SetTranslation(m_pos);
				}
				//�o�E���f�B���O�{�b�N�X�̍X�V
				m_model[m_playingAnimNum]->GetModelRender().GetSkinModel().CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);
			}

			//�ŏ��̃��[���h���W�X�V�Ȃ�...
			if (m_isFirstWorldMatRef) {
				m_isFirstWorldMatRef = false;
				//�����W�̍X�V
				m_worldMatrixOld = m_worldMatrix;
			}

			//�C���X�^���V���O���f���ɑ���
			if (m_isDraw) {
				m_model[m_playingAnimNum]->AddDrawInstance(m_worldMatrix, m_worldMatrixOld, m_SRTMatrix, m_scale, m_minAABB, m_maxAABB, m_ptrParam, m_watcher);
			}

			//�X�V���ĂȂ����
			if (!m_isUpdatedWorldMatrix) {
				m_worldMatrixOld = m_worldMatrix;
				m_isUpdatedWorldMatrix = true;//�X�V�ς݂ɂ���
			}
		}

		//���W�Ƃ��ݒ�
		void SetPos(const CVector3& pos) {
			m_isUpdatedWorldMatrix = false;
			m_pos = pos;
		}
		void SetRot(const CQuaternion& rot) {
			m_isUpdatedWorldMatrix = false; m_isSetRotOrScale = true;
			m_rot = rot;
		}
		void SetScale(const CVector3& scale) {
			m_isUpdatedWorldMatrix = false; m_isSetRotOrScale = true;
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
		//�����[���h�s��̃��Z�b�g
		void ResetWorldMatrixOld() {
			m_isFirstWorldMatRef = true;
		}

		//���f���̎擾
		InstancingModel* GetInstancingModel(int num) { return m_model[num]; }
		InstancingModel* GetInstancingModel() { return GetInstancingModel(m_playingAnimNum); }

		//�p�����[�^�̃|�C���^�ݒ�
		void SetParamPtr(void* ptrParam) {
			m_ptrParam = ptrParam;
		}		

	private:
		std::shared_ptr<InstanceWatcher> m_watcher;

		bool m_isInit = false;
		bool m_isDraw = true;
		
		std::vector<GameObj::InstancingModel*> m_model;
		int m_playingAnimNum = 0;

		bool m_isUpdatedWorldMatrix = false;//���[���h�s��X�V�ς݂�?
		bool m_isSetRotOrScale = true;		//��]�܂��͊g���ݒ肵����?
		bool m_isFirstWorldMatRef = true;	//�ŏ��̃��[���h�s��X�V��?
		CVector3 m_pos;
		CQuaternion m_rot;
		CVector3 m_scale = CVector3::One();
		CMatrix m_worldMatrix, m_worldMatrixOld;
		CMatrix m_SRTMatrix;
		CVector3 m_minAABB, m_maxAABB;

		void* m_ptrParam = nullptr;

	public:
		static InstancingModelManager& GetInstancingModelManager() { return m_s_instancingModelManager; }
	private:
		static InstancingModelManager m_s_instancingModelManager;
	};

}

	//CInstancingModelRender���Ď�����N���X
	class InstanceWatcher {
	public:
		void Watch(const GameObj::CInstancingModelRender* ptr) {
			m_render = ptr;
		}
		bool GetIsDraw() const {
			if (!m_render) { return false; }
			return m_render->GetIsDraw();
		}
	private:
		const GameObj::CInstancingModelRender* m_render = nullptr;
	};

}