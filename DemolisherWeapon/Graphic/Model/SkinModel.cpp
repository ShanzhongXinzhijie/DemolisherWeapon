#include "DWstdafx.h"
#include "SkinModel.h"
#include "SkinModelShaderConst.h"

#include <filesystem>

namespace DemolisherWeapon {

SkinModelDataManager SkinModel::m_skinModelDataManager;

SkinModel::~SkinModel()
{
	if (m_cb != nullptr) {
		//�萔�o�b�t�@������B
		m_cb->Release();
	}
	if (m_samplerState != nullptr) {
		//�T���v���X�e�[�g������B
		m_samplerState->Release();
	}
}
void SkinModel::Init(const wchar_t* filePath, EnFbxUpAxis enFbxUpAxis, EnFbxCoordinateSystem enFbxCoordinate)
{
	m_enFbxUpAxis = enFbxUpAxis;
	m_enFbxCoordinate = enFbxCoordinate;

	//�X�P���g���̃f�[�^��ǂݍ��ށB
	InitSkeleton(filePath);

	//�萔�o�b�t�@�̍쐬�B
	InitConstantBuffer();

	//�T���v���X�e�[�g�̏������B
	InitSamplerState();

	//SkinModelDataManager���g�p����CMO�t�@�C���̃��[�h�B
	m_modelDx = m_skinModelDataManager.Load(filePath, m_skeleton);	

	if (m_modelDx) {
		//�}�e���A���ݒ�̊m��
		FindMaterial(
			[&](ModelEffect* mat) {
			m_materialSetting.emplace_back();
		}
		);
	}

	//�t�@�C�����L�^
	std::experimental::filesystem::path ps = filePath;
	m_modelName = ps.stem();
}
void SkinModel::InitSkeleton(const wchar_t* filePath)
{
	//�X�P���g���̃f�[�^��ǂݍ��ށB
	//cmo�t�@�C���̊g���q��tks�ɕύX����B
	std::wstring skeletonFilePath = filePath;
	//�����񂩂�.cmo�t�@�C���n�܂�ꏊ�������B
	int pos = (int)skeletonFilePath.find(L".cmo");
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
	}
	else {
		int numBone = m_skeleton.GetNumBones();
		for (int i = 0; i < numBone; i++) {			
			m_skeleton.GetBone(i)->SetCoordinateSystem(m_enFbxUpAxis, m_enFbxCoordinate);
		}
	}
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
void SkinModel::InitSamplerState()
{
	//�e�N�X�`���̃T���v�����O���@���w�肷�邽�߂̃T���v���X�e�[�g���쐬�B
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateSamplerState(&desc, &m_samplerState);
}
void SkinModel::UpdateWorldMatrix(CVector3 position, CQuaternion rotation, CVector3 scale)
{
	CMatrix mBiasRot;
	CMatrix mBiasScr;

	CoordinateSystemBias::GetBias(mBiasRot, mBiasScr, m_enFbxUpAxis, m_enFbxCoordinate);

	CMatrix transMatrix, rotMatrix, scaleMatrix;
	//���s�ړ��s����쐬����B
	transMatrix.MakeTranslation( position );
	//��]�s����쐬����B
	rotMatrix.MakeRotationFromQuaternion( rotation );
	//rotMatrix.Mul(mBiasRot, rotMatrix);
	//�g��s����쐬����B
	scaleMatrix.MakeScaling(scale);
	//scaleMatrix.Mul(mBiasScr, scaleMatrix);

	//���[���h�s����쐬����B
	//�g��~��]�~���s�ړ��̏��Ԃŏ�Z����悤�ɁI
	//���Ԃ��ԈႦ���猋�ʂ��ς���B
	m_worldMatrix.Mul(scaleMatrix, rotMatrix);
	m_worldMatrix.Mul(m_worldMatrix, transMatrix);

	//�o�C�A�X�K��
	mBiasRot.Mul(mBiasScr, mBiasRot);
	m_worldMatrix.Mul(mBiasRot, m_worldMatrix);

	//�X�P���g���̍X�V�B
	m_skeleton.Update(m_worldMatrix);

	//�ŏ��̃��[���h���W�X�V�Ȃ�...
	if (m_isFirstWorldMatRef) {
		m_isFirstWorldMatRef = false;
		//�����W�̍X�V
		UpdateOldMatrix();
	}
}
void SkinModel::Draw(bool reverseCull)
{
	DirectX::CommonStates state(GetEngine().GetGraphicsEngine().GetD3DDevice());

	ID3D11DeviceContext* d3dDeviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "�J�������ݒ肳��Ă��܂���!!", "Error", MB_OK);
		std::abort();
	}
#endif

	//�萔�o�b�t�@�̓��e���X�V�B
	SVSConstantBuffer vsCb;
	vsCb.mWorld = m_worldMatrix;
	vsCb.mProj = GetMainCamera()->GetProjMatrix();
	vsCb.mView = GetMainCamera()->GetViewMatrix();

	vsCb.mWorld_old = m_worldMatrixOld;
	vsCb.mProj_old = GetMainCamera()->GetProjMatrixOld();
	vsCb.mView_old = GetMainCamera()->GetViewMatrixOld();

	vsCb.depthBias.x = m_depthBias;
	vsCb.depthBias.y = (GetMainCamera()->GetFar() - GetMainCamera()->GetNear())*vsCb.depthBias.x;

	d3dDeviceContext->UpdateSubresource(m_cb, 0, nullptr, &vsCb, 0, 0);

	//�萔�o�b�t�@��GPU�ɓ]���B
	d3dDeviceContext->VSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	d3dDeviceContext->PSSetConstantBuffers(enSkinModelCBReg_VSPS, 1, &m_cb);
	//�T���v���X�e�[�g��ݒ�B
	d3dDeviceContext->PSSetSamplers(0, 1, &m_samplerState);
	//�{�[���s���GPU�ɓ]���B
	m_skeleton.SendBoneMatrixArrayToGPU();

	//�}�e���A���ݒ�̓K��
	if(isMatSetInit && isMatSetEnable){
		//�ʐݒ�
		int i = 0;
		FindMaterial(
			[&](ModelEffect* mat) {
				mat->SetUseMaterialSetting(m_materialSetting[i]);
				i++;
			}
		);
	}
	else {
		//�S�̐ݒ�
		FindMaterial([&](ModelEffect* mat) { mat->SetDefaultMaterialSetting(); });
	}

	//�`��B
	m_modelDx->Draw(
		d3dDeviceContext,
		state,
		m_worldMatrix,
		GetMainCamera()->GetViewMatrix(),
		GetMainCamera()->GetProjMatrix(),
		false,
		(m_enFbxCoordinate == enFbxRightHanded) != reverseCull,
		m_pRasterizerStateCw, m_pRasterizerStateCCw
	);

	//�����W�̍X�V
	//UpdateOldMatrix();
}

}