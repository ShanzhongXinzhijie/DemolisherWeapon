#include "DWstdafx.h"
#include "CInstancingModelRender.h"
#include "Graphic/Model/SkinModelShaderConst.h"
#include "Graphic/FrustumCulling.h"

namespace DemolisherWeapon {

	namespace {
		//�L�[���쐬
		std::tuple<std::size_t, std::size_t, std::size_t> CreateInstancingModelMapKey(const wchar_t* filePath, const AnimationClip* animationClip, const wchar_t* identifier) {
			std::size_t modelHash, animHash, idenHash;
			modelHash = Util::MakeHash(filePath);
			if (animationClip) { animHash = animationClip->GetHash(); } else { animHash = Util::MakeHash(L""); }
			if (identifier) { idenHash = Util::MakeHash(identifier); } else { idenHash = Util::MakeHash(L""); }
			return std::make_tuple(modelHash, animHash, idenHash);
		}
	}

	void InstancingModelManager::Delete(const wchar_t* filePath, const AnimationClip* animationClip, const wchar_t* identifier) {
		//�L�[���쐬
		std::tuple<std::size_t, std::size_t, std::size_t> key = CreateInstancingModelMapKey(filePath, animationClip, identifier);
		//�폜
		delete m_instancingModelMap[key];
		m_instancingModelMap.erase(key);
	}

	GameObj::InstancingModel* InstancingModelManager::Load(
		int instanceMax,
		const wchar_t* filePath,
		const AnimationClip* animationClip,
		EnFbxUpAxis fbxUpAxis,
		EnFbxCoordinateSystem fbxCoordinate,
		const wchar_t* identifier,
		bool* return_isNewload 
	) {
		//�L�[���쐬
		std::tuple<std::size_t, std::size_t, std::size_t> key = CreateInstancingModelMapKey(filePath, animationClip, identifier);

		//���ɓo�^����ĂȂ���?
		if (m_instancingModelMap.count(key) > 0) {
			//�o�^����Ă���}�b�v����擾
			if (return_isNewload) { *return_isNewload = false; }
			return m_instancingModelMap[key];
		}
		else {
			//�V�K�ǂݍ���
			if (return_isNewload) { *return_isNewload = true; }
			m_instancingModelMap[key] = new GameObj::InstancingModel(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
			return m_instancingModelMap[key];
		}
	}

namespace GameObj {

	void InstancingModel::PreLoopUpdate() {
		//���[�v�O�ɃC���X�^���X���̃��Z�b�g
		//for (int i = 0; i < m_instanceIndex; i++) {//m_instanceMax
		//	m_insWatchers[i].reset();
		//}
		m_instanceIndex = 0;
	}

	//������
	void InstancingModel::Init(int instanceMax,
		const wchar_t* filePath,
		const AnimationClip* animationClip,
		EnFbxUpAxis fbxUpAxis,
		EnFbxCoordinateSystem fbxCoordinate
	) {
		//�A�j���[�V�����N���b�v�̃R�s�[
		AnimationClip* animPtr = nullptr;
		if (animationClip) { 
			m_animationClip = *animationClip; 
			animPtr = &m_animationClip;
		}
		//���f��������
		m_model.Init(filePath, animPtr, animPtr ? 1 : 0, fbxUpAxis, fbxCoordinate);
		//���[���h�s����v�Z�����Ȃ�
		m_model.GetSkinModel().SetIsCalcWorldMatrix(false);
		//������J�����O�𖳌���(�����瑤�ł��)
		m_isFrustumCull = m_model.GetSkinModel().GetIsFrustumCulling();//�����瑤�ɐݒ�
		m_model.GetSkinModel().SetIsFrustumCulling(false);
		//�C���X�^���V���O�p���_�V�F�[�_�����[�h
		if (!m_s_isShaderLoaded) {
			D3D_SHADER_MACRO macros[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
			m_s_vsShader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "INSTANCING", macros);
			m_s_vsZShader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			m_s_vsSkinShader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, "INSTANCING", macros);
			m_s_vsZSkinShader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			m_s_isShaderLoaded = true;
		}
		//�C���X�^���V���O�p���_�V�F�[�_���Z�b�g
		m_model.GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				if (mat->GetIsSkining()) {
					//�X�L�����f��
					mat->SetVS(&m_s_vsSkinShader);
					mat->SetVSZ(&m_s_vsZSkinShader);
				}
				else {
					//�X�L������Ȃ����f��
					mat->SetVS(&m_s_vsShader);
					mat->SetVSZ(&m_s_vsZShader);
				}
			}
		);
		//�J�����O�O�ɂ�鏈����ݒ�
		m_model.GetSkinModel().SetPreCullingFunction(
			[&](SkinModel*) {
				m_instanceIndex = max(0, m_instanceIndex);//0�ȉ��ɂ͂Ȃ�Ȃ�

				//�`�攻��
				int drawNum = 0;
				for (int i = 0; i < m_instanceIndex; i++) {
					//�`�悵�Ȃ�
					//if (m_insWatchers[i].expired() || !m_insWatchers[i].lock()->GetIsDraw()) { m_drawInstanceMask[i] = false; continue; }
					if (!m_isDraw[i]) { m_drawInstanceMask[i] = false; continue; }

					//������J�����O
					if (m_isFrustumCull) {
						if (!FrustumCulling::AABBTest(GetMainCamera(), m_minAABB[i], m_maxAABB[i])) {
							//�`�悵�Ȃ�
							m_drawInstanceMask[i] = false;
							continue;
						}
					}
					//�`�悷��
					m_drawInstanceMask[i] = true;
					m_worldMatrixSB.GetData()[drawNum] = m_worldMatrixCache[i];
					m_worldMatrixSBOld.GetData()[drawNum] = m_worldMatrixOldCache[i];
					drawNum++;
				}

				//�`��C���X�^���X���̐ݒ�
				m_model.GetSkinModel().SetInstanceNum(drawNum);
				m_instanceDrawNum = drawNum;
			}
		);
		//�`��O�ɂ�鏈����ݒ�
		m_model.GetSkinModel().SetPreDrawFunction(
			L"DW_InstancingModelFunc",
			[&](SkinModel*) {
				//�X�g���N�`���[�o�b�t�@�̍X�V
				m_worldMatrixSB.UpdateSubresource();
				m_worldMatrixSBOld.UpdateSubresource();

				//�V�F�[�_�[���\�[�X�Ƀ��[���h�s����Z�b�g
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrix, 1, m_worldMatrixSB.GetAddressOfSRV());
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrixOld, 1, m_worldMatrixSBOld.GetAddressOfSRV());

				//IInstanceData�̏������s
				for (auto& IID : m_instanceData) {
					IID.second->PreDraw(m_instanceIndex, m_instanceDrawNum, m_drawInstanceMask); 
				}

				//�ݒ肳��Ă��鏈�����s
				if (m_preDrawFunc) { m_preDrawFunc(); }
			}
		);

		//�ő�C���X�^���X���ݒ�
		SetInstanceMax(instanceMax);
	}

	void InstancingModel::Release() {
		m_instanceMax = 0;
		m_instanceIndex = 0; m_instanceDrawNum = 0;

		//�C���X�^���V���O�p���\�[�X�̊J��
		m_worldMatrixSB.Release();
		m_worldMatrixSBOld.Release();

		m_isDraw.reset();
		m_drawInstanceMask.reset();
		m_minAABB.reset(); m_maxAABB.reset();
		m_worldMatrixCache.reset();
		m_worldMatrixOldCache.reset();

		//m_insWatchers.reset();

		m_instanceData.clear();
	}

	void InstancingModel::SetInstanceMax(int instanceMax) {
		//IInstanceData�̏������s
		for (auto& IID : m_instanceData) {
			IID.second->SetInstanceMax(instanceMax);
		}

		//�ݒ�ő吔�����݈ȉ��Ȃ琔�����ύX
		if (m_instanceMax >= instanceMax) {
			m_instanceMax = instanceMax;
			return;
		}

		//���낢��Ċm��
		Release();
		
		//�ő�C���X�^���X��
		m_instanceMax = instanceMax;
		
		//������J�����O�p
		m_isDraw = std::make_unique<bool[]>(m_instanceMax);
		m_drawInstanceMask = std::make_unique<bool[]>(m_instanceMax);
		m_minAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_maxAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_worldMatrixCache = std::make_unique<CMatrix[]>(m_instanceMax);
		m_worldMatrixOldCache = std::make_unique<CMatrix[]>(m_instanceMax);

		//�C���X�^���X�������Ď�����
		//m_insWatchers = std::make_unique<std::weak_ptr<InstanceWatcher>[]>(m_instanceMax);
		
		//StructuredBuffer�̊m��		
		m_worldMatrixSB.Init(m_instanceMax);
		m_worldMatrixSBOld.Init(m_instanceMax);
	}
	
	CInstancingModelRender::CInstancingModelRender(bool isRegister) : IQSGameObject(isRegister) {
		//m_watcher = std::make_shared<InstanceWatcher>();
		//m_watcher->Watch(this);
	}
	CInstancingModelRender::~CInstancingModelRender() {
		//m_watcher->Watch(nullptr);
	}
	InstancingModelManager CInstancingModelRender::m_s_instancingModelManager;
}
}