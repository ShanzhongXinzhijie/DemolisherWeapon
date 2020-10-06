#include "DWstdafx.h"
#include "SkinModel.h"
#include "SkinModelShaderConst.h"
#include "Graphic/FrustumCulling.h"
#include "Model.h"
#include <filesystem>

namespace DemolisherWeapon {

SkinModelDataManager SkinModel::m_skinModelDataManager;

SkinModel::~SkinModel()
{
	if (m_cb != nullptr) {
		//�萔�o�b�t�@������B
		m_cb->Release();
	}
}
void SkinModel::Init(std::filesystem::path filePath, EnFbxUpAxis enFbxUpAxis, EnFbxCoordinateSystem enFbxCoordinate, bool isUseFlyweightFactory)
{
	if (m_model || m_modelDx) {
		DW_ERRORBOX(true,"SkinModel::Init()�����[�h�ς݂ł�")
		return;
	}

	//FBX����ݒ�
	m_enFbxUpAxis = enFbxUpAxis;
	m_enFbxCoordinate = enFbxCoordinate;

	//�o�C�A�X�s��擾
	CMatrix mBiasScr;
	CoordinateSystemBias::GetBias(m_biasMatrix, mBiasScr, m_enFbxUpAxis, m_enFbxCoordinate);
	m_biasMatrix.Mul(mBiasScr, m_biasMatrix);

	//�X�P���g���̃f�[�^��ǂݍ��ށB
	bool hasSkeleton = false;
	//if (_wcsicmp(filePath.extension().c_str(), L".tkm") != 0) {
		hasSkeleton = InitSkeleton(filePath.c_str());
	//}

	//������J�����O����
	m_isFrustumCull = true;

	//�萔�o�b�t�@�̍쐬�B
	InitConstantBuffer();

	//�g���q��tkm������
	if(_wcsicmp(filePath.extension().c_str(),L".tkm") == 0){
		//TKM�t�@�C���̃��[�h
		if (isUseFlyweightFactory) {
			//���f���v�[�����g�p
			m_model = m_skinModelDataManager.LoadCModel(filePath.c_str(), m_skeleton);
		}
		else {
			//���f����V�K�쐬
			m_modelData = m_skinModelDataManager.CreateCModel(filePath.c_str(), m_skeleton);
			m_model = m_modelData.get();
		}
	}
	else {
		//SkinModelDataManager���g�p����CMO�t�@�C���̃��[�h�B
		if (isUseFlyweightFactory) {
			//���f���v�[�����g�p
			m_modelDx = m_skinModelDataManager.Load(filePath.c_str(), m_skeleton);
		}
		else {
			//���f����V�K�쐬
			m_modelDxData = m_skinModelDataManager.CreateModel(filePath.c_str(), m_skeleton);
			m_modelDx = m_modelDxData.get();
		}
	}

	if (m_model) {
		//�o�E���f�B���O�{�b�N�X�̐���
		bool isFirst = true;
		FindMeshCModel([&](const std::unique_ptr<SModelMesh>& mesh) {
			for (int i = 0; i < mesh->m_vertexNum; i++) {
				if (isFirst) {
					m_minAABB_Origin = mesh->m_vertexData[i].position;
					m_maxAABB_Origin = mesh->m_vertexData[i].position;
					isFirst = false;
				}
				else {
					m_maxAABB_Origin.x = max(m_maxAABB_Origin.x, mesh->m_vertexData[i].position.x);
					m_maxAABB_Origin.y = max(m_maxAABB_Origin.y, mesh->m_vertexData[i].position.y);
					m_maxAABB_Origin.z = max(m_maxAABB_Origin.z, mesh->m_vertexData[i].position.z);

					m_minAABB_Origin.x = min(m_minAABB_Origin.x, mesh->m_vertexData[i].position.x);
					m_minAABB_Origin.y = min(m_minAABB_Origin.y, mesh->m_vertexData[i].position.y);
					m_minAABB_Origin.z = min(m_minAABB_Origin.z, mesh->m_vertexData[i].position.z);
				}
			}
		});
	}
	if (m_modelDx) {
		//�}�e���A���ݒ�̊m��
		/*FindMaterial(
			[&](ModelEffect* mat) {
			m_materialSetting.emplace_back();
		}
		);*/

		//�o�E���f�B���O�{�b�N�X�̎擾�E����
		bool isFirst = true;
		FindMeshes(
			[&](const std::shared_ptr<DirectX::ModelMesh>& meshes) {
				CVector3 size, extents;
				extents = meshes->boundingBox.Extents;

				//�ő�l
				size = meshes->boundingBox.Center;
				size += extents;
				if (isFirst) {
					m_maxAABB_Origin = size;
				}
				else {
					m_maxAABB_Origin.x = max(m_maxAABB_Origin.x, size.x);
					m_maxAABB_Origin.y = max(m_maxAABB_Origin.y, size.y);
					m_maxAABB_Origin.z = max(m_maxAABB_Origin.z, size.z);
				}
				//�ŏ��l
				size = meshes->boundingBox.Center;
				size -= extents;
				if (isFirst) {
					m_minAABB_Origin = size;
				}
				else {
					m_minAABB_Origin.x = min(m_minAABB_Origin.x, size.x);
					m_minAABB_Origin.y = min(m_minAABB_Origin.y, size.y);
					m_minAABB_Origin.z = min(m_minAABB_Origin.z, size.z);
				}

				isFirst = false;
			}
		);
	}
	
	//���S�ƒ[�܂ł̃x�N�g����ۑ�
	m_centerAABB = m_minAABB_Origin + m_maxAABB_Origin; m_centerAABB /= 2.0f;
	m_extentsAABB = m_maxAABB_Origin - m_centerAABB;
	//���f���{���̃o�E���f�B���O�{�b�N�X��ۑ�
	m_modelBoxCenter = m_centerAABB, m_modelBoxExtents = m_extentsAABB;
	
	//�X�L�����f���Ȃ�
	if (hasSkeleton) {
		//�傫�����{��(�A�j���[�V�������Ă����܂�T�C�Y)(�z���g�Ɏ��܂邩�͂����)
		CVector3 minBox = m_centerAABB - m_extentsAABB * 2.0f;
		CVector3 maxBox = m_centerAABB + m_extentsAABB * 2.0f;
		SetBoundingBox(minBox, maxBox);//�ݒ�
	}

	//�o�E���f�B���O�{�b�N�X������
	UpdateBoundingBoxWithWorldMatrix();

	//�t�@�C�����L�^
	m_modelName = filePath.stem();
}
bool SkinModel::InitSkeleton(const wchar_t* filePath)
{
	//�X�P���g���̃f�[�^��ǂݍ��ށB
	//�t�@�C���̊g���q��tks�ɕύX����B
	std::wstring skeletonFilePath = filePath;
	//�����񂩂�(.cmo or .tkm)���n�܂�ꏊ�������B
	int pos = (int)skeletonFilePath.find(L".cmo");
	if (pos == std::wstring::npos) {
		pos = (int)skeletonFilePath.find(L".tkm");
	}
	if (pos == std::wstring::npos) {
#ifndef DW_MASTER
		char message[256];
		sprintf_s(message, "SkinModel::InitSkeleton\nCMO�ł�TKM�ł��Ȃ�!\n%ls\n", filePath);
		MessageBox(NULL, message, "Error", MB_OK);
		//std::abort();
#endif
		return false;
	}
	//.cmo�t�@�C����.tks�ɒu��������B
	skeletonFilePath.replace(pos, 4, L".tks");
	//tks�t�@�C�������[�h����B
	bool result = m_skeleton.Load(skeletonFilePath.c_str());
	if ( result == false ) {
		//�X�P���g�����ǂݍ��݂Ɏ��s�����B
		//�A�j���[�V�������Ȃ����f���́A�X�P���g�����s�v�Ȃ̂�
		//�ǂݍ��݂Ɏ��s���邱�Ƃ͂���̂ŁA���O�o�͂����ɂ��Ă����B
#ifndef DW_MASTER
		char message[256];
		sprintf_s(message, "tks�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B%ls\n", skeletonFilePath.c_str());
		OutputDebugStringA(message);
#endif
		return false;
	}
	else {
		int numBone = m_skeleton.GetNumBones();
		for (int i = 0; i < numBone; i++) {			
			m_skeleton.GetBone(i)->SetCoordinateSystem(m_enFbxUpAxis, m_enFbxCoordinate);
		}
	}

	return true;
}
void SkinModel::InitConstantBuffer()
{
	//�쐬����o�b�t�@�̃T�C�Y��sizeof���Z�q�ŋ��߂�B
	int bufferSize = sizeof(SVSConstantBuffer);
	//�ǂ�ȃo�b�t�@���쐬����̂�������bufferDesc�ɐݒ肷��B
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));				//�O�ŃN���A�B
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;						//�o�b�t�@�őz�肳��Ă���A�ǂݍ��݂���я������ݕ��@�B
	bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;	//�o�b�t�@��16�o�C�g�A���C�����g�ɂȂ��Ă���K�v������B
																//�A���C�����g���ā��o�b�t�@�̃T�C�Y��16�̔{���Ƃ������Ƃł��B
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;			//�o�b�t�@���ǂ̂悤�ȃp�C�v���C���Ƀo�C���h���邩���w�肷��B
																//�萔�o�b�t�@�Ƀo�C���h����̂ŁAD3D11_BIND_CONSTANT_BUFFER���w�肷��B
	bufferDesc.CPUAccessFlags = 0;								//CPU �A�N�Z�X�̃t���O�ł��B
																//CPU�A�N�Z�X���s�v�ȏꍇ��0�B
	//�쐬�B
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_cb);
}

void SkinModel::UpdateWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, bool RefreshOldPos)
{
	if (m_isCalcWorldMatrix) {
		//���[���h�s����v�Z
		CalcWorldMatrix(position, rotation, scale, m_worldMatrix);
	}
	InnerUpdateWorldMatrix(m_isCalcWorldMatrix, RefreshOldPos);
}
void SkinModel::UpdateWorldMatrixTranslation(const CVector3& position, bool RefreshOldPos) 
{
	if (m_isCalcWorldMatrix) {
		//���[���h�s��̕��s�ړ�������ݒ�
		m_worldMatrix.SetTranslation(position);
	}
	InnerUpdateWorldMatrix(m_isCalcWorldMatrix, RefreshOldPos);
}

void SkinModel::SetWorldMatrix(const CMatrix& worldMatrix, bool RefreshOldPos) {
	m_worldMatrix = worldMatrix;
	InnerUpdateWorldMatrix(true, RefreshOldPos);
}

void SkinModel::InnerUpdateWorldMatrix(bool isUpdatedWorldMatrix, bool RefreshOldPos) {
	if (isUpdatedWorldMatrix) {
		//�X�P���g���̍X�V�B
		m_skeleton.Update(m_worldMatrix);
		//�o�E���f�B���O�{�b�N�X���X�V
		UpdateBoundingBoxWithWorldMatrix();
	}
	else {
		//�X�P���g���̍X�V�B
		m_skeleton.Update(CMatrix::Identity());
	}

	//�ŏ��̃��[���h���W�X�V�Ȃ�...
	if (m_isFirstWorldMatRef || RefreshOldPos) {
		m_isFirstWorldMatRef = false;
		//�����W�̍X�V
		UpdateOldMatrix();
	}
}

void SkinModel::CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix, CMatrix& returnSRTMatrix)const {
	//(�g��~��]�~���s�ړ�)�s���K��
	CalcSRTMatrix(position, rotation, scale, returnSRTMatrix);
	returnWorldMatrix = returnSRTMatrix;

	//�o�C�A�X�K��
	returnWorldMatrix.Mul(m_biasMatrix, returnWorldMatrix);
}

void SkinModel::CalcSRTMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix)const {
	//(�g��~��]�~���s�ړ�)�s����쐬
	CMatrix mat;
	returnWorldMatrix.MakeScaling(scale);			//�g��	
	mat.MakeRotationFromQuaternion(rotation);		//��]	
	returnWorldMatrix.Mul(returnWorldMatrix, mat);	//�g��~��]
	returnWorldMatrix.SetTranslation(position);		//���s�ړ���ݒ�
}

void SkinModel::CalcBoundingBoxWithWorldMatrix(const CMatrix& worldMatrix, CVector3& return_aabbMin, CVector3& return_aabbMax) {
	static const CVector3 boxOffset[8] =
	{
		{ -1.0f, -1.0f,  1.0f },
		{  1.0f, -1.0f,  1.0f },
		{  1.0f,  1.0f,  1.0f },
		{ -1.0f,  1.0f,  1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{  1.0f, -1.0f, -1.0f },
		{  1.0f,  1.0f, -1.0f },
		{ -1.0f,  1.0f, -1.0f },
	};

	//AABB�����
	CVector3 vertex;
	for (int i = 0; i < 8; i++) {
		vertex = m_centerAABB + m_extentsAABB * boxOffset[i];
		worldMatrix.Mul(vertex);
		if (i == 0) {
			return_aabbMin = vertex; return_aabbMax = vertex;
		}
		else {
			return_aabbMin.x = min(return_aabbMin.x, vertex.x); return_aabbMin.y = min(return_aabbMin.y, vertex.y); return_aabbMin.z = min(return_aabbMin.z, vertex.z);
			return_aabbMax.x = max(return_aabbMax.x, vertex.x); return_aabbMax.y = max(return_aabbMax.y, vertex.y); return_aabbMax.z = max(return_aabbMax.z, vertex.z);
		}
	}
}
void SkinModel::UpdateBoundingBoxWithWorldMatrix(){
	CalcBoundingBoxWithWorldMatrix(m_worldMatrix, m_minAABB, m_maxAABB);
}

static const float REFERENCE_FRUSTUM_SIZE = (1.0f / tan(3.14f*0.5f / 2.0f));

void SkinModel::Draw(bool reverseCull, int instanceNum, ID3D11BlendState* pBlendState, ID3D11DepthStencilState* pDepthStencilState)
{
#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "�J�������ݒ肳��Ă��܂���!!", "Error", MB_OK);
		std::abort();
		return;
	}
#endif

	//�`��t���O�����Ă��邩
	if (!GetIsDraw()) { return; }

	//���[�U�[�ݒ�̃J�����O�O�������s
	if (m_preCullingFunc) { m_preCullingFunc(this); }

	//�`��C���X�^���X����0
	if (instanceNum*m_instanceNum <= 0) { return; }

	//������J�����O
	if (m_isFrustumCull) {
		if (!FrustumCulling::AABBTest(GetMainCamera(), m_minAABB, m_maxAABB)) {
			return;
		}
	}

	ID3D11DeviceContext* d3dDeviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

	//�萔�o�b�t�@�̓��e���X�V�B
	{
		SVSConstantBuffer vsCb;
		vsCb.mWorld = m_worldMatrix;
		vsCb.mProj = GetMainCamera()->GetProjMatrix();
		vsCb.mView = GetMainCamera()->GetViewMatrix();

		vsCb.mWorld_old = m_worldMatrixOld;
		vsCb.mProj_old = GetMainCamera()->GetProjMatrixOld();
		vsCb.mView_old = GetMainCamera()->GetViewMatrixOld();

		vsCb.camMoveVec = (GetMainCamera()->GetPos() - GetMainCamera()->GetPosOld())*GetGraphicsEngine().GetMotionBlurRender().GetMotionBlurScale();
		vsCb.camMoveVec.w = GetEngine().GetDistanceScale();

		vsCb.depthBias.x = m_depthBias;
		vsCb.depthBias.y = (GetMainCamera()->GetFar() - GetMainCamera()->GetNear())*vsCb.depthBias.x;
		vsCb.depthBias.z = 50.0f*GetEngine().GetDistanceScale()*(GetMainCamera()->GetProjMatrix().m[1][1] / REFERENCE_FRUSTUM_SIZE);

		vsCb.camWorldPos = GetMainCamera()->GetPos();

		vsCb.cb_t = m_cb_t;

		//�C���|�X�^�[������
		vsCb.imposterPartNum[0] = m_imposterPartNum[0];
		vsCb.imposterPartNum[1] = m_imposterPartNum[1];
		//�C���|�X�^�[Parameter
		vsCb.imposterParameter[0] = m_imposterScale;//�X�P�[��
		vsCb.imposterParameter[1] = m_imposterRotY;//Y����]

		//�J������Near�EFar
		vsCb.nearFar.x = GetMainCamera()->GetNear();
		vsCb.nearFar.y = GetMainCamera()->GetFar();

		//�\�t�g�p�[�e�B�N�����L���ɂȂ�͈�
		vsCb.softParticleArea = m_softParticleArea;

		//���[�V�����u���[�X�P�[��
		vsCb.MotionBlurScale = GetGraphicsEngine().GetMotionBlurRender().GetMotionBlurScale();

		//�萔�o�b�t�@�X�V
		d3dDeviceContext->UpdateSubresource(m_cb, 0, nullptr, &vsCb, 0, 0);
	}

	//�萔�o�b�t�@��GPU�ɓ]���B
	d3dDeviceContext->VSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	d3dDeviceContext->PSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	//�{�[���s���GPU�ɓ]���B
	m_skeleton.SendBoneMatrixArrayToGPU();

	//�g���}�e���A���ݒ�̐ݒ�
	if(isMatSetInit && isMatSetEnable){
		//�ʐݒ�
		int i = 0;
		/*FindMaterial(
			[&](ModelEffect* mat) {
				mat->SetUseMaterialSetting(m_materialSetting[i]);
				i++;
			}
		);*/
		FindMaterialData([&](MaterialData& mat) {mat.SetUseMaterialSetting(m_materialSetting[i]); i++; });
	}
	else {
		//�S�̐ݒ�
		//FindMaterial([&](ModelEffect* mat) { mat->SetDefaultMaterialSetting(); });
		FindMaterialData([&](MaterialData& mat) {mat.SetDefaultMaterialSetting(); });
	}

	//���[�U�[�ݒ�̕`��O�������s
	for (auto& func : m_preDrawFunc) {
		func.second(this);
	}

	//�ʂ̌���
	D3D11_CULL_MODE cullMode = m_cull;
	if ((m_enFbxCoordinate == enFbxRightHanded) != reverseCull) {
		//���]
		if (cullMode == D3D11_CULL_FRONT) { 
			cullMode = D3D11_CULL_BACK; 
		}else
		if (cullMode == D3D11_CULL_BACK) { 
			cullMode = D3D11_CULL_FRONT; 
		}
	}

	//�`��
	if (m_model) {
		const DirectX::CommonStates& states = GetGraphicsEngine().GetCommonStates();

		//�A���t�@�u�����h�͂Ȃ����̂Ƃ��ĕ`��... TODO
		ID3D11BlendState* blendState = pBlendState;
		ID3D11DepthStencilState* depthStencilState = pDepthStencilState ? pDepthStencilState : m_pDepthStencilState;
		/*bool alpha = false, pmalpha = true;
		if (alpha)
		{
			if (pmalpha)
			{
				blendState = states.AlphaBlend();
				depthStencilState = states.DepthRead();
			}
			else
			{
				blendState = states.NonPremultiplied();
				depthStencilState = states.DepthRead();
			}
		}
		else*/
		{
			if (!blendState) {
				blendState = states.Opaque();
			}
			if (!depthStencilState) {
				depthStencilState = states.DepthDefault();
			}
		}
		//�u�����h�X�e�[�g�̐ݒ�
		d3dDeviceContext->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
		//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
		d3dDeviceContext->OMSetDepthStencilState(depthStencilState, 0);
		
		//���X�^���C�U�[�X�e�[�g�̐ݒ�
		bool ccw = true;//���f���̂��ꂪ�����v��肩?
		if (m_pRasterizerStateCw && m_pRasterizerStateCCw && m_pRasterizerStateNone) {
			switch (cullMode)
			{
			case D3D11_CULL_NONE:
				d3dDeviceContext->RSSetState(m_pRasterizerStateNone);
				break;
			case D3D11_CULL_FRONT:
				d3dDeviceContext->RSSetState(ccw ? m_pRasterizerStateCCw : m_pRasterizerStateCw);
				break;
			case D3D11_CULL_BACK:
				d3dDeviceContext->RSSetState(!ccw ? m_pRasterizerStateCCw : m_pRasterizerStateCw);
				break;
			default:
				break;
			}
		}
		else {
			if (false) {
				d3dDeviceContext->RSSetState(states.Wireframe());
			}
			else {
				switch (cullMode)
				{
				case D3D11_CULL_NONE:
					d3dDeviceContext->RSSetState(states.CullNone());
					break;
				case D3D11_CULL_FRONT:
					d3dDeviceContext->RSSetState(ccw ? states.CullCounterClockwise() : states.CullClockwise());
					break;
				case D3D11_CULL_BACK:
					d3dDeviceContext->RSSetState(!ccw ? states.CullCounterClockwise() : states.CullClockwise());
					break;
				default:
					break;
				}
			}
		}

		//�T���v���[�̐ݒ�
		ID3D11SamplerState* samplerState = states.AnisotropicWrap();
		d3dDeviceContext->PSSetSamplers(0, 1, &samplerState);

		//�`��
		m_model->Draw(instanceNum* m_instanceNum);
	}

#ifndef DW_DX12_TEMPORARY
	//�`��
	if (m_modelDx) {
		m_modelDx->Draw(
			d3dDeviceContext,
			GetGraphicsEngine().GetCommonStates(),
			m_worldMatrix,
			GetMainCamera()->GetViewMatrix(),
			GetMainCamera()->GetProjMatrix(),
			false,
			cullMode,
			pBlendState,
			m_pRasterizerStateCw, m_pRasterizerStateCCw, m_pRasterizerStateNone,
			pDepthStencilState ? pDepthStencilState : m_pDepthStencilState,
			instanceNum * m_instanceNum
		);
	}
#endif

	//���[�U�[�ݒ�̕`��㏈�����s
	for (auto& func : m_postDrawFunc) {
		func.second(this);
	}
}

void SkinModel::FindMeshCModel(std::function<void(const std::unique_ptr<SModelMesh>&)> onFindMesh)const
{
	if (!m_model) {
		DW_WARNING_BOX(true, "FindMesh:m_model��NULL")
			return;
	}
	m_model->FindMesh(onFindMesh);
}

void SkinModel::FindMaterialData(std::function<void(MaterialData&)> onFindMaterial) const
{
	if (m_modelDx) {
		FindMaterial([&](ModelEffect* mat) {
			onFindMaterial(mat->GetMatrialData());
		});
	}
	if (m_model) {
		FindMeshCModel([&](const std::unique_ptr<SModelMesh>& mesh) {
			for (auto& mat : mesh->m_materials) {
				onFindMaterial(mat->GetMaterialData());
			}
			});
	}
}

}