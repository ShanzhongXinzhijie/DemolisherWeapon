#include "DWstdafx.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "Skeleton.h"
#include "Model.h"

namespace DemolisherWeapon {

std::unique_ptr<DirectX::Model> SkinModelDataManager::CreateModel(const wchar_t* filePath, const Skeleton& skeleton)
{
#ifndef DW_DX12_TEMPORARY

	//�{�[���𔭌������Ƃ��̃R�[���o�b�N�֐��B
	auto onFindBone = [&](
		const wchar_t* boneName,
		const VSD3DStarter::Bone* bone,
		std::vector<int>& localBoneIDtoGlobalBoneIDTbl
		)
	{
		int globalBoneID = skeleton.FindBoneID(boneName);
		if (globalBoneID == -1) {
			//�{�[����������Ȃ������B
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "�{�[����������܂���B\ntks�t�@�C������܂���?\n%ls\n", filePath);
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			return;
		}
		localBoneIDtoGlobalBoneIDTbl.push_back(globalBoneID);
	};

	//�G�t�F�N�g�t�@�N�g���B
	SkinModelEffectFactory effectFactory(GetEngine().GetGraphicsEngine().GetD3DDevice());
	//�e�N�X�`��������t�H���_��ݒ肷��B
	effectFactory.SetDirectory(L"Resource/modelData");

	//CMO�t�@�C���̃��[�h�B
#ifndef DW_MASTER
	try {
#endif
		std::unique_ptr<DirectX::Model> model = DirectX::Model::CreateFromCMO(	//CMO�t�@�C�����烂�f�����쐬����֐��́ACreateFromCMO�����s����B
			GetEngine().GetGraphicsEngine().GetD3DDevice(),			//��������D3D�f�o�C�X�B
			filePath,									//�������͓ǂݍ���CMO�t�@�C���̃t�@�C���p�X�B
			effectFactory,								//��O�����̓G�t�F�N�g�t�@�N�g���B
			false,										//��l������Cull���[�h�B���͋C�ɂ��Ȃ��Ă悢�B
			false,
			onFindBone
		);

		return model;

#ifndef DW_MASTER
	}
	catch (std::exception& exception) {
		// �G���[����
		char message[256];
		if (strcmp(exception.what(), "CreateFromCMO") == 0) {
			sprintf_s(message, "cmo�t�@�C���̃��[�h�Ɏ��s�B\n�t�@�C���p�X�����Ă܂���?\n%ls\n", filePath);
		}
		else {
			sprintf_s(message, "cmo�t�@�C���̃��[�h�Ɏ��s�B\n%ls\n", filePath);
		}
		MessageBox(NULL, message, "Error", MB_OK);

		HRESULT hr = GetGraphicsEngine().GetD3DDevice()->GetDeviceRemovedReason();
		std::abort();
	}
#endif

#else
	return {};
#endif
}
std::unique_ptr<CModel> SkinModelDataManager::CreateCModel(const wchar_t* filePath, const Skeleton& skeleton) {
	//�p�X��char�֕ϊ�...
	size_t iReturnValue;
	size_t size = wcslen(filePath) + 1;
	std::unique_ptr<char[]> charPath = std::make_unique<char[]>(size);
	errno_t err = wcstombs_s(
		&iReturnValue,
		charPath.get(),
		size, //��̃T�C�Y
		filePath,
		size - 1 //�R�s�[����ő啶����
	);
	if (err != 0) {
		DW_ERRORBOX(true, "wcstombs_s errno:%d", err)
	}

	//�쐬
	std::unique_ptr<CModel> model = std::make_unique<CModel>();
	model->LoadTkmFile(charPath.get());

	if (model->IsInited()) {
		model->CreateMeshParts();
		return model;
	}
	else {
		return {};
	}
}

DirectX::Model* SkinModelDataManager::Load(const wchar_t* filePath, const Skeleton& skeleton)
{
	DirectX::Model* retModel = NULL;//�߂�l	

	//�}�b�v�ɓo�^����Ă��邩���ׂ�B
	auto it = m_directXModelMap.find(filePath);
	if (it == m_directXModelMap.end()) {
		auto model = CreateModel(filePath, skeleton);
		//���o�^�Ȃ̂ŁA�V�K�Ń��[�h����B
		retModel = model.get();
		//�G���[
		if (!retModel) { return retModel; }
		//�V�K�Ȃ̂Ń}�b�v�ɓo�^����B
		m_directXModelMap.insert({ filePath, std::move(model) });
	}
	else {
		//�o�^����Ă���̂ŁA�ǂݍ��ݍς݂̃f�[�^���擾�B
		retModel = it->second.get();
	}

	return retModel;
}
CModel* SkinModelDataManager::LoadCModel(const wchar_t* filePath, const Skeleton& skeleton)
{
	CModel* retModel = nullptr;//�߂�l	

	//�}�b�v�ɓo�^����Ă��邩���ׂ�B
	auto it = m_cmodelMap.find(filePath);
	if (it == m_cmodelMap.end()) {
		auto model = CreateCModel(filePath, skeleton);
		//���o�^�Ȃ̂ŁA�V�K�Ń��[�h����B
		retModel = model.get();
		//�G���[
		if (!retModel) { return retModel; }
		//�V�K�Ȃ̂Ń}�b�v�ɓo�^����B
		m_cmodelMap.insert({ filePath, std::move(model) });
	}
	else {
		//�o�^����Ă���̂ŁA�ǂݍ��ݍς݂̃f�[�^���擾�B
		retModel = it->second.get();
	}

	return retModel;
}

void SkinModelDataManager::Release()
{
	//map����ɂ���B
	m_directXModelMap.clear();
	m_cmodelMap.clear();
}

}