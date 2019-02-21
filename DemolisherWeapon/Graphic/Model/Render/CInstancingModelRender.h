#pragma once

namespace DemolisherWeapon {

namespace GameObj {

	//�C���X�^���V���O�p���f��
	class InstancingModel : public IQSGameObject {
	public:
		InstancingModel() {};
		InstancingModel(int instanceMax,
			const wchar_t* filePath,
			AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		) {
			Init(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
		}
		~InstancingModel() {
			Release();
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
			AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded
		) {			
			//���f��������
			m_model.Init(filePath, animationClip, animationClip ? 1 : 0, fbxUpAxis, fbxCoordinate);
			//���[���h�s����v�Z�����Ȃ�
			m_model.GetSkinModel().SetIsCalcWorldMatrix(false);
			//�C���X�^���V���O�p���_�V�F�[�_�����[�h
			D3D_SHADER_MACRO macros[] = { "INSTANCING", "1", NULL, NULL };
			m_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "INSTANCING", macros);
			m_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			m_vsSkinShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, "INSTANCING", macros);
			m_vsZSkinShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			//�C���X�^���V���O�p���_�V�F�[�_���Z�b�g
			m_model.GetSkinModel().FindMaterialSetting(
				[&](MaterialSetting* mat) {
					if (mat->GetModelEffect()->GetIsSkining()) {
						//�X�L�����f��
						mat->SetVS(&m_vsSkinShader);
						mat->SetVSZ(&m_vsZSkinShader);
					}
					else {
						//�X�L������Ȃ����f��
						mat->SetVS(&m_vsShader);
						mat->SetVSZ(&m_vsZShader);
					}
				}
			);

			//�ő�C���X�^���X���ݒ�
			SetInstanceMax(instanceMax);
		}

		//�ő�C���X�^���X���̐ݒ�
		void SetInstanceMax(int instanceMax);

		//���f���̎擾
		GameObj::CSkinModelRender& GetModelRender() { return m_model; }

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

		void PostLoopPostUpdate()override;

	private:
		int m_instanceNum = 0;
		int m_instanceMax = 0;

		GameObj::CSkinModelRender m_model;

		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrix;
		ID3D11Buffer*				m_worldMatrixSB = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRV = nullptr;

		std::unique_ptr<CMatrix[]>	m_instancingWorldMatrixOld;
		ID3D11Buffer*				m_worldMatrixSBOld = nullptr;
		ID3D11ShaderResourceView*	m_worldMatrixSRVOld = nullptr;

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

		GameObj::InstancingModel* Load(
			int instanceMax,										//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t* filePath,
			AnimationClip* animationClip = nullptr,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//�V�K�ǂݍ��ݎ��̂ݎg�p
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t* identifier = nullptr
		);
	private:
		typedef std::tuple<int, int, int> key_t;

		struct key_hash 
		{
			std::size_t operator()(const key_t& k) const
			{
				return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
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
		void Init(int instanceMax,									//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t* filePath,
			AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ,					//�V�K�ǂݍ��ݎ��̂ݎg�p
			EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded,	//�V�K�ǂݍ��ݎ��̂ݎg�p
			const wchar_t* identifier = nullptr
		) {
			for (int i = 0; i < max(numAnimationClips,1); i++) {
				m_model = m_s_instancingModelManager.Load(instanceMax, filePath, &animationClips[i], fbxUpAxis, fbxCoordinate, identifier);
			}
		}

		//CSkinModelRender& GetCSkinModelRender(int num) { return m_modelRender[num]; }

		//���[���h�s������߂�(�o�C�A�X�܂�)
		//�C���X�^���X�ɑ���
		void PostLoopUpdate()override final {
			m_model->GetModelRender().GetSkinModel().CalcWorldMatrix( m_pos, m_rot, m_scale, m_worldMatrix);

			//�ŏ��̃��[���h���W�X�V�Ȃ�...
			if (m_isFirstWorldMatRef) {
				m_isFirstWorldMatRef = false;
				//�����W�̍X�V
				m_worldMatrixOld = m_worldMatrix;
			}

			m_model->AddDrawInstance(&m_worldMatrix, &m_worldMatrixOld);
			m_worldMatrixOld = m_worldMatrix;
		}

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

	private:
		GameObj::InstancingModel* m_model;

		bool m_isFirstWorldMatRef = true;
		CVector3 m_pos;
		CQuaternion m_rot;
		CVector3 m_scale = CVector3::One();
		CMatrix m_worldMatrix, m_worldMatrixOld;

		static InstancingModelManager m_s_instancingModelManager;
	};

}
}