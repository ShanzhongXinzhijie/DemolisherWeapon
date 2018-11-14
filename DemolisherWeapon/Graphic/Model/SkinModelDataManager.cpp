#include "DWstdafx.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "Skeleton.h"

namespace DemolisherWeapon {

DirectX::Model* SkinModelDataManager::Load(const wchar_t* filePath, const Skeleton& skeleton)
{
	DirectX::Model* retModel = NULL;
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
	//�}�b�v�ɓo�^����Ă��邩���ׂ�B
	auto it = m_directXModelMap.find(filePath);
	if (it == m_directXModelMap.end()) {
		//���o�^�Ȃ̂ŁA�V�K�Ń��[�h����B
		//�G�t�F�N�g�t�@�N�g���B
		SkinModelEffectFactory effectFactory(GetEngine().GetGraphicsEngine().GetD3DDevice());
		//�e�N�X�`��������t�H���_��ݒ肷��B
		effectFactory.SetDirectory(L"Resource/modelData");
		//CMO�t�@�C���̃��[�h�B
		try {
			auto model = DirectX::Model::CreateFromCMO(	//CMO�t�@�C�����烂�f�����쐬����֐��́ACreateFromCMO�����s����B
				GetEngine().GetGraphicsEngine().GetD3DDevice(),			//��������D3D�f�o�C�X�B
				filePath,									//�������͓ǂݍ���CMO�t�@�C���̃t�@�C���p�X�B
				effectFactory,								//��O�����̓G�t�F�N�g�t�@�N�g���B
				false,										//��l������Cull���[�h�B���͋C�ɂ��Ȃ��Ă悢�B
				false,
				onFindBone
			);		
			retModel = model.get();
			//�V�K�Ȃ̂Ń}�b�v�ɓo�^����B
			m_directXModelMap.insert({ filePath, std::move(model) });
		}
		catch (std::exception& exception) {
			// �G���[����
#ifndef DW_MASTER
			char message[256];
			if (strcmp(exception.what() , "CreateFromCMO")==0) {
				sprintf_s(message, "cmo�t�@�C���̃��[�h�Ɏ��s�B\n�t�@�C���p�X�����Ă܂���?\n%ls\n", filePath);
			}
			else {
				sprintf_s(message, "cmo�t�@�C���̃��[�h�Ɏ��s�B\n%ls\n", filePath);
			}
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
		}
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
}

}