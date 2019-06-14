#include "DWstdafx.h"
#include "CInstancingModelRender.h"
#include "Graphic/Model/SkinModelShaderConst.h"

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
		//�`��O�ɂ�鏈����ݒ�
		m_model.GetSkinModel().SetPreDrawFunction(
			[&](SkinModel*) {
				//�V�F�[�_�[���\�[�X�Ƀ��[���h�s����Z�b�g
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrix, 1, &m_worldMatrixSRV);
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrixOld, 1, &m_worldMatrixSRVOld);

				//IInstanceData�̏������s
				if (m_instanceData) { m_instanceData->PreDrawUpdate(); }

				//�ݒ肳��Ă��鏈�����s
				if (m_preDrawFunc) { m_preDrawFunc(); }
			}
		);

		//�ő�C���X�^���X���ݒ�
		SetInstanceMax(instanceMax);
	}

	void InstancingModel::SetInstanceMax(int instanceMax) {
		//IInstanceData�̏������s
		if (m_instanceData) { m_instanceData->SetInstanceMax(instanceMax); }

		//�ݒ�ő吔�����݈ȉ��Ȃ琔�����ύX
		if (m_instanceMax >= instanceMax) {
			m_instanceMax = instanceMax;
			return;
		}

		//���낢��Ċm��
		Release();
		
		m_instanceMax = instanceMax;
		
		//���[���h�s��̊m��
		m_instancingWorldMatrix.reset(new CMatrix[m_instanceMax]);
		m_instancingWorldMatrixOld.reset(new CMatrix[m_instanceMax]);

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

	void InstancingModel::PostLoopPostUpdate() {

		m_instanceNum = max(0, m_instanceNum);

		//�`��C���X�^���X���̐ݒ�
		m_model.GetSkinModel().SetInstanceNum(m_instanceNum);
		m_instanceDrawNum = m_instanceNum;

		if (m_instanceNum <= 0) { return; }		

		//StructuredBuffer���X�V�B
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_worldMatrixSB, 0, NULL, m_instancingWorldMatrix.get(), 0, 0
		);
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_worldMatrixSBOld, 0, NULL, m_instancingWorldMatrixOld.get(), 0, 0
		);

		//IInstanceData�̏������s
		if (m_instanceData) { m_instanceData->PostLoopPostUpdate(); }
		
		m_instanceNum = 0;
	}
	
	InstancingModelManager CInstancingModelRender::m_s_instancingModelManager;
}
}