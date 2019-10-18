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
		const wchar_t* identifier
	) {
		//�L�[���쐬
		std::tuple<std::size_t, std::size_t, std::size_t> key = CreateInstancingModelMapKey(filePath, animationClip, identifier);

		//���ɓo�^����ĂȂ���?
		if (m_instancingModelMap.count(key) > 0) {
			//�o�^����Ă���}�b�v����擾
			return m_instancingModelMap[key];
		}
		else {
			//�V�K�ǂݍ���
			m_instancingModelMap[key] = new GameObj::InstancingModel(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
			return m_instancingModelMap[key];
		}
	}

namespace GameObj {

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
		D3D_SHADER_MACRO macros[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
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
		//�J�����O�O�ɂ�鏈����ݒ�
		m_model.GetSkinModel().SetPreCullingFunction(
			[&](SkinModel*) {
				m_instanceIndex = max(0, m_instanceIndex);

				//������J�����O
				int drawNum = 0;
				for (int i = 0; i < m_instanceIndex; i++) {
					if (m_isFrustumCull) {
						if (!FrustumCulling::AABBTest(GetMainCamera(), m_minAABB[i], m_maxAABB[i])) {
							//�`�悵�Ȃ�
							m_drawInstanceMask[i] = false;
							continue;
						}
					}
					//�`�悷��
					m_drawInstanceMask[i] = true;
					m_instancingWorldMatrix[drawNum] = m_worldMatrixCache[i];
					m_instancingWorldMatrixOld[drawNum] = m_worldMatrixOldCache[i];
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
				GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
					m_worldMatrixSB, 0, NULL, m_instancingWorldMatrix.get(), 0, 0
				);
				GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
					m_worldMatrixSBOld, 0, NULL, m_instancingWorldMatrixOld.get(), 0, 0
				);

				//�V�F�[�_�[���\�[�X�Ƀ��[���h�s����Z�b�g
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrix, 1, &m_worldMatrixSRV);
				GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrixOld, 1, &m_worldMatrixSRVOld);

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
		
		m_instanceMax = instanceMax;
		
		//���[���h�s��̊m��
		m_instancingWorldMatrix = std::make_unique<CMatrix[]>(m_instanceMax);
		m_instancingWorldMatrixOld = std::make_unique<CMatrix[]>(m_instanceMax);
		//������J�����O�p
		m_drawInstanceMask = std::make_unique<bool[]>(m_instanceMax);
		m_minAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_maxAABB = std::make_unique<CVector3[]>(m_instanceMax);
		m_worldMatrixCache = std::make_unique<CMatrix[]>(m_instanceMax);
		m_worldMatrixOldCache = std::make_unique<CMatrix[]>(m_instanceMax);
		
		//StructuredBuffer�̊m��
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(CMatrix);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					
		desc.ByteWidth = static_cast<UINT>(stride * m_instanceMax);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_worldMatrixSB);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_worldMatrixSBOld);
		
		//ShaderResourceView�̊m��
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_worldMatrixSB, &descSRV, &m_worldMatrixSRV);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_worldMatrixSBOld, &descSRV, &m_worldMatrixSRVOld);
	}
	
	InstancingModelManager CInstancingModelRender::m_s_instancingModelManager;
}
}